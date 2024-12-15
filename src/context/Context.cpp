#include "Context.hpp"

#include "../log/Exception.hpp"
#include "coContext/coroutine/BasePromise.hpp"
#include "coContext/ring/SubmissionQueueEntry.hpp"

coContext::Context::Context() :
    ring{[] {
        io_uring_params parameters{};
        parameters.flags = IORING_SETUP_SUBMIT_ALL | IORING_SETUP_COOP_TASKRUN | IORING_SETUP_TASKRUN_FLAG |
                           IORING_SETUP_SINGLE_ISSUER | IORING_SETUP_DEFER_TASKRUN;

        const std::lock_guard lock{mutex};

        if (sharedRingFileDescriptor != -1) {
            parameters.flags |= IORING_SETUP_ATTACH_WQ;
            parameters.wq_fd = sharedRingFileDescriptor;
        }

        Ring ring{static_cast<std::uint32_t>(getFileDescriptorLimit()) * 2, parameters};

        if (sharedRingFileDescriptor == -1) sharedRingFileDescriptor = ring.getFileDescriptor();

        return ring;
    }()} {
    constexpr cpu_set_t cpuSet{};
    {
        const std::lock_guard lock{mutex};

        CPU_SET(cpuCode++, std::addressof(cpuSet));
        cpuCode %= std::thread::hardware_concurrency();
    }
    this->ring.registerCpuAffinity(sizeof(cpuSet), std::addressof(cpuSet));

    this->ring.registerSelfFileDescriptor();

    this->schedulingCoroutines.emplace(std::hash<Coroutine>{}(Coroutine{}), Coroutine{});
}

auto coContext::Context::swap(Context &other) noexcept -> void {
    std::swap(this->isRunning, other.isRunning);
    std::swap(this->ring, other.ring);
    std::swap(this->unscheduledCoroutines, other.unscheduledCoroutines);
    std::swap(this->schedulingCoroutines, other.schedulingCoroutines);
}

auto coContext::Context::run() -> void {
    this->isRunning = true;

    this->scheduleUnscheduledCoroutines();

    while (this->isRunning) {
        this->ring.submitAndWait(1);
        this->ring.advance(this->ring.poll([this](const io_uring_cqe *const completionQueueEntry) {
            this->scheduleCoroutine(this->schedulingCoroutines.at(completionQueueEntry->user_data),
                                    completionQueueEntry->res);
        }));

        this->scheduleUnscheduledCoroutines();
    }
}

auto coContext::Context::stop() noexcept -> void { this->isRunning = false; }

auto coContext::Context::spawn(Coroutine &&coroutine) -> void {
    this->unscheduledCoroutines.emplace(std::move(coroutine));
}

auto coContext::Context::getSubmissionQueueEntry() -> SubmissionQueueEntry {
    return SubmissionQueueEntry{this->ring.getSubmissionQueueEntry()};
}

auto coContext::Context::syncCancel(const std::variant<std::uint64_t, std::int32_t> identity, const std::int32_t flags,
                                    const __kernel_timespec timeSpecification) -> std::int32_t {
    io_uring_sync_cancel_reg parameters{};

    if (std::holds_alternative<std::uint64_t>(identity)) parameters.addr = std::get<std::uint64_t>(identity);
    else {
        parameters.fd = std::get<std::int32_t>(identity);
        parameters.flags = IORING_ASYNC_CANCEL_FD;
    }

    parameters.flags |= flags;
    parameters.timeout = timeSpecification;

    return this->ring.syncCancel(parameters);
}

auto coContext::Context::getFileDescriptorLimit(const std::source_location sourceLocation) -> rlim_t {
    rlimit limit{};
    if (getrlimit(RLIMIT_NOFILE, std::addressof(limit)) == -1) {
        throw Exception{
            Log{Log::Level::fatal,
                std::pmr::string{std::error_code{errno, std::generic_category()}.message(), getMemoryResource()},
                sourceLocation}
        };
    }

    return limit.rlim_cur;
}

auto coContext::Context::scheduleUnscheduledCoroutines() -> void {
    while (!std::empty(this->unscheduledCoroutines)) {
        this->scheduleCoroutine(this->unscheduledCoroutines.front());

        this->unscheduledCoroutines.pop();
    }
}

auto coContext::Context::scheduleCoroutine(Coroutine &coroutine, const std::int32_t result) -> void {
    if (!static_cast<bool>(coroutine)) return;

    coroutine.promise().setResult(result);
    coroutine();

    if (const std::uint64_t coroutineIdentity{std::hash<Coroutine>{}(coroutine)}; !coroutine.done()) {
        Coroutine childCoroutine{std::move(coroutine.promise().getChildCoroutine())};

        this->schedulingCoroutines.try_emplace(coroutineIdentity, std::move(coroutine));

        this->scheduleCoroutine(childCoroutine);
    } else {
        this->schedulingCoroutines.erase(coroutineIdentity);

        this->scheduleCoroutine(this->schedulingCoroutines.at(coroutine.promise().getParentCoroutineIdentity()));
    }
}

std::mutex coContext::Context::mutex;
std::int32_t coContext::Context::sharedRingFileDescriptor{-1};
std::uint32_t coContext::Context::cpuCode;

#include "Context.hpp"

#include "../log/Exception.hpp"
#include "coContext/coroutine/GenericTask.hpp"
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
}

auto coContext::Context::swap(Context &other) noexcept -> void {
    std::swap(this->isRunning, other.isRunning);
    std::swap(this->ring, other.ring);
    std::swap(this->unscheduledTasks, other.unscheduledTasks);
    std::swap(this->schedulingTasks, other.schedulingTasks);
}

auto coContext::Context::spawn(GenericTask &&task) -> void { this->unscheduledTasks.emplace(std::move(task)); }

auto coContext::Context::run() -> void {
    this->isRunning = true;
    this->scheduleTasks();

    while (this->isRunning) {
        this->ring.submitAndWait(1);
        this->ring.advance(this->ring.poll([this](const io_uring_cqe *const completionQueueEntry) {
            if (const auto findResult{this->schedulingTasks->find(completionQueueEntry->user_data)};
                findResult != std::cend(*this->schedulingTasks)) {
                GenericTask &task{findResult->second};
                const Coroutine &coroutine{task.getCoroutine()};

                task.setResult(completionQueueEntry->res);
                coroutine();

                if (coroutine.done()) this->schedulingTasks->erase(findResult);
            }
        }));

        this->scheduleTasks();
    }
}

auto coContext::Context::stop() noexcept -> void { this->isRunning = false; }

auto coContext::Context::getConstSchedulingTasks() const noexcept
    -> std::shared_ptr<const std::unordered_map<std::uint64_t, GenericTask>> {
    return this->schedulingTasks;
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
            Log{Log::Level::fatal, std::error_code{errno, std::generic_category()}.message(), sourceLocation}
        };
    }

    return limit.rlim_cur;
}

auto coContext::Context::scheduleTasks() -> void {
    while (!std::empty(this->unscheduledTasks)) {
        GenericTask &task{this->unscheduledTasks.front()};
        const Coroutine &coroutine{task.getCoroutine()};

        coroutine();
        if (!coroutine.done()) this->schedulingTasks->emplace(std::hash<Coroutine>{}(coroutine), std::move(task));

        this->unscheduledTasks.pop();
    }
}

std::mutex coContext::Context::mutex;
std::int32_t coContext::Context::sharedRingFileDescriptor{-1};
std::uint32_t coContext::Context::cpuCode;

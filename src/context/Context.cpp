#include "Context.hpp"

#include "../log/Exception.hpp"
#include "coContext/coroutine/BasePromise.hpp"
#include "coContext/ring/Submission.hpp"

#include <sys/resource.h>

coContext::internal::Context::Context() {
    this->ring->registerSelfFileDescriptor();

    this->ring->registerSparseFileDescriptor(
        [](const std::source_location sourceLocation = std::source_location::current()) {
            rlimit limit{};
            if (getrlimit(RLIMIT_NOFILE, std::addressof(limit)) == -1) {
                throw Exception{
                    Log{Log::Level::fatal,
                        std::pmr::string{std::error_code{errno, std::generic_category()}.message(),
                                         getSyncMemoryResource()},
                        sourceLocation}
                };
            }

            return limit.rlim_cur;
        }());
}

auto coContext::internal::Context::swap(Context &other) noexcept -> void {
    std::swap(this->ring, other.ring);
    std::swap(this->ringBuffer, other.ringBuffer);
    std::swap(this->unscheduledCoroutines, other.unscheduledCoroutines);
    std::swap(this->schedulingCoroutines, other.schedulingCoroutines);
    std::swap(this->isRunning, other.isRunning);
}

auto coContext::internal::Context::getRingBuffer() noexcept -> RingBuffer & { return this->ringBuffer; }

auto coContext::internal::Context::run() -> void {
    this->isRunning = true;

    this->scheduleUnscheduledCoroutines();

    while (this->isRunning) {
        this->ring->submitAndWait(1);
        this->ringBuffer.advance(this->ring->poll([this](const io_uring_cqe *const completion) {
            const auto result{this->schedulingCoroutines.find(completion->user_data)};
            Coroutine coroutine{std::move(result->second)};
            this->schedulingCoroutines.erase(result);

            coroutine.getPromise().setResult(completion->res);
            coroutine.getPromise().setFlags(completion->flags);

            this->scheduleCoroutine(std::move(coroutine));
        }));

        this->scheduleUnscheduledCoroutines();
    }
}

auto coContext::internal::Context::stop() noexcept -> void { this->isRunning = false; }

auto coContext::internal::Context::spawn(Coroutine coroutine) -> void {
    this->unscheduledCoroutines.emplace(std::move(coroutine));
}

auto coContext::internal::Context::getSubmission() const -> Submission {
    return Submission{this->ring->getSubmission()};
}

auto coContext::internal::Context::syncCancel(const std::variant<std::uint64_t, std::int32_t> id,
                                              const std::int32_t flags, const __kernel_timespec timeSpecification) const
    -> std::int32_t {
    io_uring_sync_cancel_reg parameters{};

    if (std::holds_alternative<std::uint64_t>(id)) parameters.addr = std::get<std::uint64_t>(id);
    else {
        parameters.fd = std::get<std::int32_t>(id);
        parameters.flags = IORING_ASYNC_CANCEL_FD;
    }

    parameters.flags |= flags;
    parameters.timeout = timeSpecification;

    return this->ring->syncCancel(parameters);
}

auto coContext::internal::Context::scheduleUnscheduledCoroutines() -> void {
    while (!std::empty(this->unscheduledCoroutines)) {
        Coroutine coroutine{std::move(this->unscheduledCoroutines.front())};
        this->unscheduledCoroutines.pop();

        this->scheduleCoroutine(std::move(coroutine));
    }
}

auto coContext::internal::Context::scheduleCoroutine(Coroutine coroutine) -> void {
    coroutine();

    if (!coroutine.isDone()) {
        Coroutine childCoroutine{std::move(coroutine.getPromise().getChildCoroutine())};

        const std::uint64_t id{std::hash<Coroutine>{}(coroutine)};
        this->schedulingCoroutines.emplace(id, std::move(coroutine));

        if (static_cast<bool>(childCoroutine)) this->scheduleCoroutine(std::move(childCoroutine));
    } else if (const auto result{this->schedulingCoroutines.find(coroutine.getPromise().getParentCoroutineId())};
               result != std::cend(this->schedulingCoroutines)) {
        Coroutine parentCoroutine{std::move(result->second)};
        this->schedulingCoroutines.erase(result);

        this->scheduleCoroutine(std::move(parentCoroutine));
    }
}

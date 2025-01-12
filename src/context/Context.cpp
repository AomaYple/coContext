#include "Context.hpp"

#include "../log/Exception.hpp"
#include "coContext/coroutine/BasePromise.hpp"
#include "coContext/log/logger.hpp"
#include "coContext/ring/Submission.hpp"

#include <sys/resource.h>

using namespace std::string_view_literals;

coContext::internal::Context::Context() {
    try {
        this->ring->registerSelfFileDescriptor();
    } catch (Exception &exception) { logger::write(Log{std::move(exception.getLog())}); }

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

auto coContext::internal::Context::run(const std::source_location sourceLocation) -> void {
    logger::write(Log{
        Log::Level::info, std::pmr::string{"context running"sv, getSyncMemoryResource()},
         sourceLocation
    });

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

    logger::write(Log{
        Log::Level::info, std::pmr::string{"context stopped"sv, getSyncMemoryResource()},
         sourceLocation
    });
}

auto coContext::internal::Context::stop(const std::source_location sourceLocation) -> void {
    this->isRunning = false;

    logger::write(Log{
        Log::Level::info, std::pmr::string{"context stopping"sv, getSyncMemoryResource()},
         sourceLocation
    });
}

auto coContext::internal::Context::spawn(Coroutine coroutine) -> void {
    this->unscheduledCoroutines.emplace(std::move(coroutine));
}

auto coContext::internal::Context::getSubmission() const -> Submission {
    Submission submission{nullptr};
    try {
        submission = Submission{this->ring->getSubmission()};
    } catch (Exception &exception) {
        logger::write(Log{std::move(exception.getLog())});

        this->ring->submit();
        submission = Submission{this->ring->getSubmission()};
    }

    return submission;
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

    std::int32_t result;
    try {
        result = this->ring->syncCancel(parameters);
    } catch (Exception &exception) {
        result = -std::stoi(std::string{exception.getLog().getMessage()});

        logger::write(Log{std::move(exception.getLog())});
    }

    return result;
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
    } else if (const std::exception_ptr exceptionPointer{*coroutine.getPromise().getExceptionPointer()};
               static_cast<bool>(exceptionPointer))
        std::rethrow_exception(exceptionPointer);
}

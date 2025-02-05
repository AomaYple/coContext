#include "Context.hpp"

#include "../log/Exception.hpp"
#include "coContext/coroutine/BasePromise.hpp"
#include "coContext/log/logger.hpp"

#include <sys/resource.h>

using namespace std::string_view_literals;

coContext::internal::Context::Context() {
    try {
        this->ring->registerSelfFileDescriptor();
    } catch (Exception &exception) { logger::write(Log{std::move(exception.getLog())}); }

    this->ring->registerSparseFileDescriptor(
        [](const std::source_location sourceLocation = std::source_location::current()) constexpr {
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
    std::swap(this->bufferRing, other.bufferRing);
    std::swap(this->unscheduledCoroutines, other.unscheduledCoroutines);
    std::swap(this->schedulingCoroutines, other.schedulingCoroutines);
    std::swap(this->isRunning, other.isRunning);
}

auto coContext::internal::Context::getBufferRing() noexcept -> BufferRing & { return this->bufferRing; }

auto coContext::internal::Context::run(const std::source_location sourceLocation) -> void {
    this->isRunning = true;

    logger::write(Log{
        Log::Level::info, std::pmr::string{"context running"sv, getSyncMemoryResource()},
         sourceLocation
    });

    this->scheduleUnscheduledCoroutines();

    while (this->isRunning) {
        this->ring->submitAndWait(1);
        this->bufferRing.advance(this->ring->poll([this](const io_uring_cqe &completion) constexpr {
            const auto result{this->schedulingCoroutines.find(completion.user_data)};
            Coroutine coroutine{std::move(result->second)};
            this->schedulingCoroutines.erase(result);

            coroutine.getPromise().setResult(completion.res);
            coroutine.getPromise().setFlags(completion.flags);

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
    this->unscheduledCoroutines.emplace_back(std::move(coroutine));
}

auto coContext::internal::Context::getSubmission() const -> io_uring_sqe * {
    try {
        return this->ring->getSubmission();
    } catch (Exception &exception) {
        logger::write(Log{std::move(exception.getLog())});

        this->ring->submit();

        return this->ring->getSubmission();
    }
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

    try {
        return this->ring->syncCancel(std::addressof(parameters));
    } catch (Exception &exception) {
        const std::int32_t result{-std::stoi(std::string{exception.getLog().getMessage()})};

        logger::write(Log{std::move(exception.getLog())});

        return result;
    }
}

auto coContext::internal::Context::scheduleUnscheduledCoroutines() -> void {
    for (std::size_t i{}; i != std::size(this->unscheduledCoroutines); ++i)
        this->scheduleCoroutine(std::move(this->unscheduledCoroutines[i]));

    this->unscheduledCoroutines.clear();
}

auto coContext::internal::Context::scheduleCoroutine(Coroutine coroutine) -> void {
    do {
        coroutine();

        if (!coroutine.isDone()) {
            Coroutine childCoroutine{std::move(coroutine.getPromise().getChildCoroutine())};

            const std::uint64_t id{std::hash<Coroutine>{}(coroutine)};
            this->schedulingCoroutines.emplace(id, std::move(coroutine));

            coroutine = std::move(childCoroutine);
        } else if (const auto result{this->schedulingCoroutines.find(coroutine.getPromise().getParentCoroutineId())};
                   result != std::cend(this->schedulingCoroutines)) {
            Coroutine parentCoroutine{std::move(result->second)};
            this->schedulingCoroutines.erase(result);

            coroutine = std::move(parentCoroutine);
        } else if (const std::exception_ptr exception{*coroutine.getPromise().getException()}; exception) [[unlikely]] {
            std::rethrow_exception(exception);
        } else coroutine = Coroutine{nullptr};
    } while (coroutine);
}

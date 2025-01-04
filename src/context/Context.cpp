#include "Context.hpp"

#include "../log/Exception.hpp"
#include "../ring/Ring.hpp"
#include "coContext/coroutine/BasePromise.hpp"
#include "coContext/ring/Submission.hpp"

#include <sys/resource.h>

coContext::internal::Context::Context() :
    ring{[] {
        io_uring_params parameters{};
        parameters.flags = IORING_SETUP_SUBMIT_ALL | IORING_SETUP_COOP_TASKRUN | IORING_SETUP_TASKRUN_FLAG |
                           IORING_SETUP_SINGLE_ISSUER | IORING_SETUP_DEFER_TASKRUN;

        return std::allocate_shared<Ring>(std::pmr::polymorphic_allocator{getMemoryResource()}, entries, parameters);
    }()},
    ringBuffer{this->ring, entries, 0, IOU_PBUF_RING_INC} {
    this->ring->registerSelfFileDescriptor();

    this->ring->registerSparseFileDescriptor([](const std::source_location sourceLocation =
                                                    std::source_location::current()) {
        rlimit limit{};
        if (getrlimit(RLIMIT_NOFILE, std::addressof(limit)) == -1) {
            throw Exception{
                Log{Log::Level::fatal,
                    std::pmr::string{std::error_code{errno, std::generic_category()}.message(), getMemoryResource()},
                    sourceLocation}
            };
        }

        return limit.rlim_cur;
    }());
}

auto coContext::internal::Context::swap(Context &other) noexcept -> void {
    std::swap(this->ring, other.ring);
    std::swap(this->ringBuffer, other.ringBuffer);
    std::swap(this->bufferGroup, other.bufferGroup);
    std::swap(this->unscheduledCoroutines, other.unscheduledCoroutines);
    std::swap(this->schedulingCoroutines, other.schedulingCoroutines);
    std::swap(this->isRunning, other.isRunning);
}

auto coContext::internal::Context::run() -> void {
    this->isRunning = true;

    this->scheduleUnscheduledCoroutines();

    while (this->isRunning) {
        this->ring->submitAndWait(1);
        this->ringBuffer.advance(this->ring->poll([this](const io_uring_cqe *const completion) {
            const auto findResult{this->schedulingCoroutines.find(completion->user_data)};
            Coroutine coroutine{std::move(findResult->second)};
            this->schedulingCoroutines.erase(findResult);

            coroutine.promise().setResult(completion->res);
            coroutine.promise().setFlags(completion->flags);

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

auto coContext::internal::Context::syncCancel(const std::variant<std::uint64_t, std::int32_t> identity,
                                              const std::int32_t flags, const __kernel_timespec timeSpecification) const
    -> std::int32_t {
    io_uring_sync_cancel_reg parameters{};

    if (std::holds_alternative<std::uint64_t>(identity)) parameters.addr = std::get<std::uint64_t>(identity);
    else {
        parameters.fd = std::get<std::int32_t>(identity);
        parameters.flags = IORING_ASYNC_CANCEL_FD;
    }

    parameters.flags |= flags;
    parameters.timeout = timeSpecification;

    return this->ring->syncCancel(parameters);
}

auto coContext::internal::Context::getRingBufferId() const noexcept -> std::int32_t { return this->ringBuffer.getId(); }

auto coContext::internal::Context::getData(const std::uint16_t bufferId, const std::size_t dataSize) noexcept
    -> std::span<const std::byte> {
    auto &[buffer, offset]{this->bufferGroup[bufferId]};
    this->ringBuffer.addBuffer(buffer, bufferId);

    const std::span data{std::cbegin(buffer) + static_cast<std::ptrdiff_t>(offset), dataSize};
    offset += dataSize;

    return data;
}

auto coContext::internal::Context::clearBufferOffset(const std::uint16_t bufferId) noexcept -> void {
    this->bufferGroup[bufferId].offset = 0;
}

auto coContext::internal::Context::expandBuffer() -> void {
    if (std::size(this->bufferGroup) == entries) return;

    this->bufferGroup.emplace_back();
    this->ringBuffer.addBuffer(this->bufferGroup.back().buffer, std::size(this->bufferGroup) - 1);
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

    if (!coroutine.done()) {
        Coroutine childCoroutine{std::move(coroutine.promise().getChildCoroutine())};

        const std::uint64_t identity{std::hash<Coroutine>{}(coroutine)};
        this->schedulingCoroutines.emplace(identity, std::move(coroutine));

        if (static_cast<bool>(childCoroutine)) this->scheduleCoroutine(std::move(childCoroutine));
    } else if (const auto findResult{this->schedulingCoroutines.find(coroutine.promise().getParentCoroutineIdentity())};
               findResult != std::cend(this->schedulingCoroutines)) {
        Coroutine parentCoroutine{std::move(findResult->second)};
        this->schedulingCoroutines.erase(findResult);

        this->scheduleCoroutine(std::move(parentCoroutine));
    }
}

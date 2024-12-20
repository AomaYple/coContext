#pragma once

#include "../ring/SubmissionQueueEntry.hpp"
#include "Coroutine.hpp"

#include <memory>

namespace coContext {
    class AsyncWaiter {
        using TimeSpecification = std::pair<std::unique_ptr<__kernel_timespec>, std::unique_ptr<__kernel_timespec>>;

    public:
        explicit AsyncWaiter(SubmissionQueueEntry submissionQueueEntry) noexcept;

        AsyncWaiter(const AsyncWaiter &) = delete;

        auto operator=(const AsyncWaiter &) -> AsyncWaiter & = delete;

        constexpr AsyncWaiter(AsyncWaiter &&) noexcept = default;

        constexpr auto operator=(AsyncWaiter &&) noexcept -> AsyncWaiter & = default;

        constexpr ~AsyncWaiter() = default;

        auto swap(AsyncWaiter &other) noexcept -> void;

        [[nodiscard]] auto getSubmissionQueueEntry() const noexcept -> SubmissionQueueEntry;

        [[nodiscard]] auto getTimeSpecifications() noexcept -> TimeSpecification &;

        [[nodiscard]] auto await_ready() const noexcept -> bool;

        auto await_suspend(std::coroutine_handle<> genericCoroutineHandle) noexcept -> void;

        [[nodiscard]] auto await_resume() const -> std::int32_t;

        [[nodiscard]] auto getAsyncWaitResumeFlags() const -> std::uint32_t;

    private:
        SubmissionQueueEntry submissionQueueEntry;
        Coroutine::Handle coroutineHandle;
        TimeSpecification timeSpecifications;
    };
}    // namespace coContext

template<>
constexpr auto std::swap(coContext::AsyncWaiter &lhs, coContext::AsyncWaiter &rhs) noexcept -> void {
    lhs.swap(rhs);
}

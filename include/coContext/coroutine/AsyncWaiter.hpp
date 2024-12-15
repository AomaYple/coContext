#pragma once

#include "../ring/SubmissionQueueEntry.hpp"

#include <chrono>
#include <coroutine>

namespace coContext {
    class AsyncWaiter {
    public:
        explicit AsyncWaiter(SubmissionQueueEntry submissionQueueEntry) noexcept;

        AsyncWaiter(const AsyncWaiter &) = delete;

        auto operator=(const AsyncWaiter &) -> AsyncWaiter & = delete;

        AsyncWaiter(AsyncWaiter &&) noexcept = default;

        auto operator=(AsyncWaiter &&) noexcept -> AsyncWaiter & = default;

        ~AsyncWaiter() = default;

        auto swap(AsyncWaiter &other) noexcept -> void;

        [[nodiscard]] auto getSubmissionQueueEntry() const noexcept -> SubmissionQueueEntry;

        [[nodiscard]] auto getFirstTimeSpecification() const noexcept -> __kernel_timespec;

        [[nodiscard]] auto getFirstTimeSpecification() noexcept -> __kernel_timespec &;

        auto setFirstTimeSpecification(std::chrono::seconds seconds, std::chrono::nanoseconds nanoseconds) -> void;

        [[nodiscard]] auto getSecondTimeSpecification() const noexcept -> __kernel_timespec;

        [[nodiscard]] auto getSecondTimeSpecification() noexcept -> __kernel_timespec &;

        auto setSecondTimeSpecification(std::chrono::seconds seconds, std::chrono::nanoseconds nanoseconds) -> void;

        [[nodiscard]] auto await_ready() const noexcept -> bool;

        auto await_suspend(std::coroutine_handle<> genericCoroutineHandle) -> void;

        [[nodiscard]] auto await_resume() const noexcept -> std::int32_t;

    private:
        SubmissionQueueEntry submissionQueueEntry;
        std::shared_ptr<std::int32_t> result;
        std::pair<__kernel_timespec, __kernel_timespec> timeSpecifications;
    };
}    // namespace coContext

template<>
constexpr auto std::swap(coContext::AsyncWaiter &lhs, coContext::AsyncWaiter &rhs) noexcept -> void {
    lhs.swap(rhs);
}

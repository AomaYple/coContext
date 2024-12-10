#pragma once

#include "../ring/SubmissionQueueEntry.hpp"

#include <coroutine>
#include <memory>
#include <unordered_map>

namespace coContext {
    class GenericTask;

    class AsyncWaiter {
        using Tasks = std::shared_ptr<const std::unordered_map<std::uint64_t, GenericTask>>;

    public:
        AsyncWaiter(Tasks tasks, SubmissionQueueEntry submissionQueueEntry) noexcept;

        AsyncWaiter(const AsyncWaiter &) = delete;

        auto operator=(const AsyncWaiter &) = delete;

        AsyncWaiter(AsyncWaiter &&) noexcept = default;

        auto operator=(AsyncWaiter &&) noexcept -> AsyncWaiter & = default;

        ~AsyncWaiter() = default;

        auto swap(AsyncWaiter &other) noexcept -> void;

        [[nodiscard]] auto getSubmissionQueueEntry() const noexcept -> SubmissionQueueEntry;

        [[nodiscard]] auto getTimeSpecification() const -> __kernel_timespec &;

        auto setTimeSpecification(__kernel_timespec timeSpecification) -> void;

        auto setTimeoutAsyncWaiter(AsyncWaiter &&timeoutAsyncWaiter) -> void;

        [[nodiscard]] auto await_ready() const noexcept -> bool;

        auto await_suspend(std::coroutine_handle<> handle) noexcept -> void;

        [[nodiscard]] auto await_resume() const -> std::int32_t;

    private:
        Tasks tasks;
        SubmissionQueueEntry submissionQueueEntry;
        std::uint64_t taskIdentity{};
        std::unique_ptr<__kernel_timespec> timeSpecification;
        std::unique_ptr<AsyncWaiter> timeoutAsyncWaiter;
    };
}    // namespace coContext

template<>
constexpr auto std::swap(coContext::AsyncWaiter &lhs, coContext::AsyncWaiter &rhs) noexcept -> void {
    lhs.swap(rhs);
}

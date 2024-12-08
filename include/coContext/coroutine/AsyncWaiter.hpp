#pragma once

#include "../ring/SubmissionQueueEntry.hpp"

#include <coroutine>
#include <memory>
#include <unordered_map>

namespace coContext {
    class GenericTask;

    class AsyncWaiter {
        using TaskMap = std::shared_ptr<const std::unordered_map<std::uint64_t, GenericTask>>;

        friend auto operator==(const AsyncWaiter &lhs, const AsyncWaiter &rhs) noexcept -> bool;

    public:
        explicit AsyncWaiter(TaskMap tasks = {},
                             SubmissionQueueEntry submissionQueueEntry = SubmissionQueueEntry{}) noexcept;

        AsyncWaiter(const AsyncWaiter &) noexcept = default;

        auto operator=(const AsyncWaiter &) noexcept -> AsyncWaiter & = default;

        AsyncWaiter(AsyncWaiter &&) noexcept = default;

        auto operator=(AsyncWaiter &&) noexcept -> AsyncWaiter & = default;

        ~AsyncWaiter() = default;

        auto swap(AsyncWaiter &other) noexcept -> void;

        auto setTimeSpecification(__kernel_timespec timeSpecification) noexcept -> void;

        [[nodiscard]] auto getTimeSpecification() noexcept -> __kernel_timespec &;

        [[nodiscard]] auto await_ready() const noexcept -> bool;

        auto await_suspend(std::coroutine_handle<> handle) noexcept -> void;

        [[nodiscard]] auto await_resume() const -> std::int32_t;

    private:
        TaskMap tasks;
        SubmissionQueueEntry submissionQueueEntry;
        std::uint64_t taskIdentity{};
        __kernel_timespec timeSpecification{};
    };

    [[nodiscard]] auto operator==(const AsyncWaiter &lhs, const AsyncWaiter &rhs) noexcept -> bool;
}    // namespace coContext

template<>
constexpr auto std::swap(coContext::AsyncWaiter &lhs, coContext::AsyncWaiter &rhs) noexcept -> void {
    lhs.swap(rhs);
}

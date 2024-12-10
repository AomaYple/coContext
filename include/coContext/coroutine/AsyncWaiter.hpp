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
        explicit AsyncWaiter(Tasks tasks = {},
                             SubmissionQueueEntry submissionQueueEntry = SubmissionQueueEntry{}) noexcept;

        AsyncWaiter(const AsyncWaiter &) noexcept = default;

        auto operator=(const AsyncWaiter &) noexcept -> AsyncWaiter & = default;

        AsyncWaiter(AsyncWaiter &&) noexcept = default;

        auto operator=(AsyncWaiter &&) noexcept -> AsyncWaiter & = default;

        ~AsyncWaiter() = default;

        auto swap(AsyncWaiter &other) noexcept -> void;

        [[nodiscard]] auto getTasks() const noexcept -> Tasks;

        [[nodiscard]] auto getSubmissionQueueEntry() const noexcept -> SubmissionQueueEntry;

        [[nodiscard]] auto getTaskIdentity() const noexcept -> std::uint64_t;

        [[nodiscard]] auto getTimeSpecification() const noexcept -> __kernel_timespec;

        [[nodiscard]] auto getTimeSpecification() noexcept -> __kernel_timespec &;

        auto setTimeSpecification(__kernel_timespec timeSpecification) noexcept -> void;

        [[nodiscard]] auto await_ready() const noexcept -> bool;

        auto await_suspend(std::coroutine_handle<> handle) noexcept -> void;

        [[nodiscard]] auto await_resume() const -> std::int32_t;

    private:
        Tasks tasks;
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

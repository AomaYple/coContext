#pragma once

#include "../ring/SubmissionQueueEntry.hpp"

#include <chrono>
#include <coroutine>
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

        [[nodiscard]] auto getFirstTimeSpecification() const noexcept -> __kernel_timespec;

        [[nodiscard]] auto getFirstTimeSpecification() noexcept -> __kernel_timespec &;

        auto setFirstTimeSpecification(std::chrono::seconds seconds, std::chrono::nanoseconds nanoseconds) -> void;

        [[nodiscard]] auto getSecondTimeSpecification() const noexcept -> __kernel_timespec;

        [[nodiscard]] auto getSecondTimeSpecification() noexcept -> __kernel_timespec &;

        auto setSecondTimeSpecification(std::chrono::seconds seconds, std::chrono::nanoseconds nanoseconds) -> void;

        [[nodiscard]] auto await_ready() const noexcept -> bool;

        auto await_suspend(std::coroutine_handle<> handle) noexcept -> void;

        [[nodiscard]] auto await_resume() const -> std::int32_t;

    private:
        Tasks tasks;
        SubmissionQueueEntry submissionQueueEntry;
        std::uint64_t taskIdentity{};
        std::pair<__kernel_timespec, __kernel_timespec> timeSpecifications;
    };

    [[nodiscard]] auto operator==(const AsyncWaiter &lhs, const AsyncWaiter &rhs) noexcept -> bool;
}    // namespace coContext

template<>
constexpr auto std::swap(coContext::AsyncWaiter &lhs, coContext::AsyncWaiter &rhs) noexcept -> void {
    lhs.swap(rhs);
}

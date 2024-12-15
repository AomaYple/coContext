#pragma once

#include "../ring/SubmissionQueueEntry.hpp"

#include <chrono>
#include <coroutine>

namespace coContext {
    class AsyncWaiter {
    public:
        explicit AsyncWaiter(SubmissionQueueEntry submissionQueueEntry = SubmissionQueueEntry{}) noexcept;

        AsyncWaiter(const AsyncWaiter &) noexcept = default;

        auto operator=(const AsyncWaiter &) noexcept -> AsyncWaiter & = default;

        AsyncWaiter(AsyncWaiter &&) noexcept = default;

        auto operator=(AsyncWaiter &&) noexcept -> AsyncWaiter & = default;

        ~AsyncWaiter() = default;

        [[nodiscard]] auto getSubmissionQueueEntry() const noexcept -> SubmissionQueueEntry;

        [[nodiscard]] auto getResult() const noexcept -> const std::shared_ptr<std::int32_t> &;

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

    [[nodiscard]] auto operator==(const AsyncWaiter &, const AsyncWaiter &) noexcept -> bool;
}    // namespace coContext

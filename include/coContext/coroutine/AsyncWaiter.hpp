#pragma once

#include "../ring/SubmissionQueueEntry.hpp"
#include "Coroutine.hpp"

#include <chrono>

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

        [[nodiscard]] auto getCoroutineHandle() const noexcept -> Coroutine::Handle;

        [[nodiscard]] auto getFirstTimeSpecification() const noexcept -> __kernel_timespec;

        [[nodiscard]] auto getFirstTimeSpecification() noexcept -> __kernel_timespec &;

        auto setFirstTimeSpecification(std::chrono::seconds seconds, std::chrono::nanoseconds nanoseconds) -> void;

        [[nodiscard]] auto getSecondTimeSpecification() const noexcept -> __kernel_timespec;

        [[nodiscard]] auto getSecondTimeSpecification() noexcept -> __kernel_timespec &;

        auto setSecondTimeSpecification(std::chrono::seconds seconds, std::chrono::nanoseconds nanoseconds) -> void;

        [[nodiscard]] auto await_ready() const noexcept -> bool;

        auto await_suspend(std::coroutine_handle<> genericCoroutineHandle) noexcept -> void;

        [[nodiscard]] auto await_resume() const -> std::int32_t;

    private:
        SubmissionQueueEntry submissionQueueEntry;
        Coroutine::Handle coroutineHandle;
        std::pair<__kernel_timespec, __kernel_timespec> timeSpecifications;
    };

    [[nodiscard]] auto operator==(const AsyncWaiter &, const AsyncWaiter &) noexcept -> bool;
}    // namespace coContext

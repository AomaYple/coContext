#pragma once

#include "../ring/SubmissionQueueEntry.hpp"

#include <chrono>
#include <coroutine>

namespace coContext {
    class BasePromise;

    class AsyncWaiter {
        using Coroutine = std::coroutine_handle<BasePromise>;

    public:
        explicit AsyncWaiter(SubmissionQueueEntry submissionQueueEntry = SubmissionQueueEntry{}) noexcept;

        constexpr AsyncWaiter(const AsyncWaiter &) noexcept = default;

        constexpr auto operator=(const AsyncWaiter &) noexcept -> AsyncWaiter & = default;

        constexpr AsyncWaiter(AsyncWaiter &&) noexcept = default;

        constexpr auto operator=(AsyncWaiter &&) noexcept -> AsyncWaiter & = default;

        constexpr ~AsyncWaiter() = default;

        [[nodiscard]] auto getSubmissionQueueEntry() const noexcept -> SubmissionQueueEntry;

        [[nodiscard]] auto getCoroutine() const noexcept -> Coroutine;

        [[nodiscard]] auto getFirstTimeSpecification() const noexcept -> __kernel_timespec;

        [[nodiscard]] auto getFirstTimeSpecification() noexcept -> __kernel_timespec &;

        auto setFirstTimeSpecification(std::chrono::seconds seconds, std::chrono::nanoseconds nanoseconds) -> void;

        [[nodiscard]] auto getSecondTimeSpecification() const noexcept -> __kernel_timespec;

        [[nodiscard]] auto getSecondTimeSpecification() noexcept -> __kernel_timespec &;

        auto setSecondTimeSpecification(std::chrono::seconds seconds, std::chrono::nanoseconds nanoseconds) -> void;

        [[nodiscard]] auto await_ready() const noexcept -> bool;

        auto await_suspend(std::coroutine_handle<> coroutine) noexcept -> void;

        [[nodiscard]] auto await_resume() const -> std::int32_t;

    private:
        SubmissionQueueEntry submissionQueueEntry;
        Coroutine coroutine{Coroutine::from_address(std::noop_coroutine().address())};
        std::pair<__kernel_timespec, __kernel_timespec> timeSpecifications;
    };

    [[nodiscard]] auto operator==(const AsyncWaiter &lhs, const AsyncWaiter &rhs) noexcept -> bool;
}    // namespace coContext

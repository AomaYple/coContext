#pragma once

#include "../ring/Submission.hpp"
#include "Coroutine.hpp"

namespace coContext::internal {
    class AsyncWaiter {
    public:
        explicit AsyncWaiter(Submission submission) noexcept;

        AsyncWaiter(const AsyncWaiter &) = delete;

        auto operator=(const AsyncWaiter &) -> AsyncWaiter & = delete;

        constexpr AsyncWaiter(AsyncWaiter &&) noexcept = default;

        constexpr auto operator=(AsyncWaiter &&) noexcept -> AsyncWaiter & = default;

        constexpr ~AsyncWaiter() = default;

        auto swap(AsyncWaiter &other) noexcept -> void;

        [[nodiscard]] auto getSubmission() const noexcept -> Submission;

        [[nodiscard]] auto getTimeSpecification() const noexcept -> const std::unique_ptr<__kernel_timespec> &;

        auto setTimeSpecification(std::unique_ptr<__kernel_timespec> timeSpecification) noexcept -> void;

        [[nodiscard]] auto await_ready() const noexcept -> bool;

        auto await_suspend(std::coroutine_handle<> genericCoroutineHandle) noexcept -> void;

        [[nodiscard]] auto await_resume() const -> std::int32_t;

        [[nodiscard]] auto getAsyncWaitResumeFlags() const -> std::uint32_t;

    private:
        Submission submission;
        Coroutine::Handle coroutineHandle;
        std::unique_ptr<__kernel_timespec> timeSpecification;
    };
}    // namespace coContext::internal

template<>
constexpr auto std::swap(coContext::internal::AsyncWaiter &lhs, coContext::internal::AsyncWaiter &rhs) noexcept
    -> void {
    lhs.swap(rhs);
}

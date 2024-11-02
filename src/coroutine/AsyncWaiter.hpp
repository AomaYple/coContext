#pragma once

#include "Task.hpp"

namespace coContext {
    class AsyncWaiter {
    public:
        explicit AsyncWaiter(const Submission &submission) noexcept;

        [[nodiscard]] constexpr auto await_ready() const noexcept { return false; }

        auto await_suspend(std::coroutine_handle<Task::promise_type> handle) -> void;

        [[nodiscard]] auto await_resume() const -> Outcome;

    private:
        std::coroutine_handle<Task::promise_type> handle;
        Submission submission;
    };
}    // namespace coContext

#pragma once

#include "Task.hpp"

namespace coContext {
    class AsyncWaiter {
    public:
        constexpr AsyncWaiter() noexcept = default;

        AsyncWaiter(const AsyncWaiter &) = delete;

        auto operator=(const AsyncWaiter &) -> AsyncWaiter & = delete;

        AsyncWaiter(AsyncWaiter &&) noexcept = default;

        auto operator=(AsyncWaiter &&) noexcept -> AsyncWaiter & = default;

        ~AsyncWaiter() = default;

        [[nodiscard]] auto await_ready() const noexcept -> bool;

        auto await_suspend(std::coroutine_handle<Task::promise_type> handle) -> void;

        [[nodiscard]] auto await_resume() const -> int;

    private:
        std::coroutine_handle<Task::promise_type> handle;
    };
}    // namespace coContext

#include "coContext/coroutine/AsyncWaiter.hpp"

auto coContext::AsyncWaiter::await_ready() const noexcept -> bool { return false; }

auto coContext::AsyncWaiter::await_suspend(const std::coroutine_handle<Task::promise_type> handle) -> void {
    this->handle = handle;
}

auto coContext::AsyncWaiter::await_resume() const -> int { return this->handle.promise().getResult(); }

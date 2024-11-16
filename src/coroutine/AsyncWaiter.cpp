#include "coContext/coroutine/AsyncWaiter.hpp"

coContext::AsyncWaiter::AsyncWaiter(const Submission &submission) noexcept : submission{submission} {}

auto coContext::AsyncWaiter::await_suspend(const std::coroutine_handle<Task::promise_type> handle) -> void {
    this->handle = handle;
    this->submission.setUserData(std::hash<std::coroutine_handle<Task::promise_type>>{}(this->handle));
    this->handle.promise().setSubmission(this->submission);
}

auto coContext::AsyncWaiter::await_resume() const -> int { return this->handle.promise().getResult(); }

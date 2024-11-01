#include "Awaiter.hpp"

coContext::Awaiter::Awaiter(const Submission &submission) noexcept : submission{submission} {}

auto coContext::Awaiter::await_suspend(const std::coroutine_handle<Task::promise_type> handle) -> void {
    this->handle = handle;
    this->submission.userData = std::hash<std::coroutine_handle<Task::promise_type>>{}(this->handle);
    this->handle.promise().setSubmission(this->submission);
}

auto coContext::Awaiter::await_resume() const -> Outcome { return this->handle.promise().getOutcome(); }

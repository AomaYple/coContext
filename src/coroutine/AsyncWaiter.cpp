#include "coContext/coroutine/AsyncWaiter.hpp"

#include "coContext/coroutine/GenericTask.hpp"

coContext::AsyncWaiter::AsyncWaiter(Tasks tasks, const SubmissionQueueEntry submissionQueueEntry) noexcept :
    tasks{std::move(tasks)}, submissionQueueEntry{submissionQueueEntry} {}

auto coContext::AsyncWaiter::swap(AsyncWaiter &other) noexcept -> void {
    std::swap(this->tasks, other.tasks);
    std::swap(this->submissionQueueEntry, other.submissionQueueEntry);
    std::swap(this->taskIdentity, other.taskIdentity);
    std::swap(this->timeSpecification, other.timeSpecification);
    std::swap(this->timeoutAsyncWaiter, other.timeoutAsyncWaiter);
}

auto coContext::AsyncWaiter::getSubmissionQueueEntry() const noexcept -> SubmissionQueueEntry {
    return this->submissionQueueEntry;
}

auto coContext::AsyncWaiter::getTimeSpecification() const -> __kernel_timespec & { return *this->timeSpecification; }

auto coContext::AsyncWaiter::setTimeSpecification(const __kernel_timespec timeSpecification) -> void {
    this->timeSpecification = std::make_unique<__kernel_timespec>(timeSpecification);
}

auto coContext::AsyncWaiter::setTimeoutAsyncWaiter(AsyncWaiter &&timeoutAsyncWaiter) -> void {
    this->timeoutAsyncWaiter = std::make_unique<AsyncWaiter>(std::move(timeoutAsyncWaiter));
}

auto coContext::AsyncWaiter::await_ready() const noexcept -> bool { return {}; }

auto coContext::AsyncWaiter::await_suspend(const std::coroutine_handle<> handle) noexcept -> void {
    this->taskIdentity = std::hash<std::coroutine_handle<>>{}(handle);
    this->submissionQueueEntry.setUserData(this->taskIdentity);
}

auto coContext::AsyncWaiter::await_resume() const -> std::int32_t {
    return this->tasks->at(this->taskIdentity).getResult();
}

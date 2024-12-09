#include "coContext/coroutine/AsyncWaiter.hpp"

#include "coContext/coroutine/GenericTask.hpp"

coContext::AsyncWaiter::AsyncWaiter(Tasks tasks, const SubmissionQueueEntry submissionQueueEntry) noexcept :
    tasks{std::move(tasks)}, submissionQueueEntry{submissionQueueEntry} {}

auto coContext::AsyncWaiter::swap(AsyncWaiter &other) noexcept -> void {
    std::swap(this->tasks, other.tasks);
    std::swap(this->submissionQueueEntry, other.submissionQueueEntry);
    std::swap(this->taskIdentity, other.taskIdentity);
    std::swap(this->timeSpecification, other.timeSpecification);
}

auto coContext::AsyncWaiter::setTimeSpecification(const __kernel_timespec timeSpecification) noexcept -> void {
    this->timeSpecification = timeSpecification;
}

auto coContext::AsyncWaiter::getTimeSpecification() noexcept -> __kernel_timespec & { return this->timeSpecification; }

auto coContext::AsyncWaiter::await_ready() const noexcept -> bool { return {}; }

auto coContext::AsyncWaiter::await_suspend(const std::coroutine_handle<> handle) noexcept -> void {
    this->taskIdentity = std::hash<std::coroutine_handle<>>{}(handle);
    this->submissionQueueEntry.setUserData(this->taskIdentity);
}

auto coContext::AsyncWaiter::await_resume() const -> std::int32_t {
    return this->tasks->at(this->taskIdentity).getResult();
}

auto coContext::operator==(const AsyncWaiter &lhs, const AsyncWaiter &rhs) noexcept -> bool {
    return lhs.tasks == rhs.tasks && lhs.submissionQueueEntry == rhs.submissionQueueEntry &&
           lhs.taskIdentity == rhs.taskIdentity && lhs.timeSpecification.tv_sec == rhs.timeSpecification.tv_sec &&
           lhs.timeSpecification.tv_nsec == rhs.timeSpecification.tv_nsec;
}

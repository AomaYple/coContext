#include "coContext/coroutine/AsyncWaiter.hpp"

#include "coContext/coroutine/GenericTask.hpp"

coContext::AsyncWaiter::AsyncWaiter(Tasks tasks, const SubmissionQueueEntry submissionQueueEntry) noexcept :
    tasks{std::move(tasks)}, submissionQueueEntry{submissionQueueEntry} {}

auto coContext::AsyncWaiter::swap(AsyncWaiter &other) noexcept -> void {
    std::swap(this->tasks, other.tasks);
    std::swap(this->submissionQueueEntry, other.submissionQueueEntry);
    std::swap(this->taskIdentity, other.taskIdentity);
    std::swap(this->timeSpecifications, other.timeSpecifications);
}

auto coContext::AsyncWaiter::getSubmissionQueueEntry() const noexcept -> SubmissionQueueEntry {
    return this->submissionQueueEntry;
}

auto coContext::AsyncWaiter::setFirstTimeSpecification(const std::chrono::seconds seconds,
                                                       const std::chrono::nanoseconds nanoseconds) -> void {
    this->timeSpecifications.first.tv_sec = seconds.count();
    this->timeSpecifications.first.tv_nsec = nanoseconds.count();
}

auto coContext::AsyncWaiter::setSecondTimeSpecification(const std::chrono::seconds seconds,
                                                        const std::chrono::nanoseconds nanoseconds) -> void {
    this->timeSpecifications.second.tv_sec = seconds.count();
    this->timeSpecifications.second.tv_nsec = nanoseconds.count();
}

auto coContext::AsyncWaiter::getFirstTimeSpecification() noexcept -> __kernel_timespec & {
    return this->timeSpecifications.first;
}

auto coContext::AsyncWaiter::getSecondTimeSpecification() noexcept -> __kernel_timespec & {
    return this->timeSpecifications.second;
}

auto coContext::AsyncWaiter::await_ready() const noexcept -> bool { return {}; }

auto coContext::AsyncWaiter::await_suspend(const std::coroutine_handle<> handle) noexcept -> void {
    this->taskIdentity = std::hash<std::coroutine_handle<>>{}(handle);
    this->submissionQueueEntry.setUserData(this->taskIdentity);
}

auto coContext::AsyncWaiter::await_resume() const -> std::int32_t {
    return this->tasks->at(this->taskIdentity).getResult();
}

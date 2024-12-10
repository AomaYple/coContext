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

auto coContext::AsyncWaiter::getTasks() const noexcept -> Tasks { return this->tasks; }

auto coContext::AsyncWaiter::getSubmissionQueueEntry() const noexcept -> SubmissionQueueEntry {
    return this->submissionQueueEntry;
}

auto coContext::AsyncWaiter::getTaskIdentity() const noexcept -> std::uint64_t { return this->taskIdentity; }

auto coContext::AsyncWaiter::getTimeSpecification() const noexcept -> __kernel_timespec {
    return this->timeSpecification;
}

auto coContext::AsyncWaiter::getTimeSpecification() noexcept -> __kernel_timespec & { return this->timeSpecification; }

auto coContext::AsyncWaiter::setTimeSpecification(const __kernel_timespec timeSpecification) noexcept -> void {
    this->timeSpecification = timeSpecification;
}

auto coContext::AsyncWaiter::await_ready() const noexcept -> bool { return {}; }

auto coContext::AsyncWaiter::await_suspend(const std::coroutine_handle<> handle) noexcept -> void {
    this->taskIdentity = std::hash<std::coroutine_handle<>>{}(handle);
    this->submissionQueueEntry.setUserData(this->taskIdentity);
}

auto coContext::AsyncWaiter::await_resume() const -> std::int32_t {
    return this->tasks->at(this->taskIdentity).getResult();
}

auto coContext::operator==(const AsyncWaiter &lhs, const AsyncWaiter &rhs) noexcept -> bool {
    const __kernel_timespec lhsTimeSpecification{lhs.getTimeSpecification()},
        rhsTimeSpecification{rhs.getTimeSpecification()};

    return lhs.getTasks() == rhs.getTasks() && lhs.getSubmissionQueueEntry() == rhs.getSubmissionQueueEntry() &&
           lhs.getTaskIdentity() == rhs.getTaskIdentity() &&
           lhsTimeSpecification.tv_sec == rhsTimeSpecification.tv_sec &&
           lhsTimeSpecification.tv_nsec == rhsTimeSpecification.tv_nsec;
}

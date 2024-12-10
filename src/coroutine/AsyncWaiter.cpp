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

auto coContext::AsyncWaiter::getTasks() const noexcept -> Tasks { return this->tasks; }

auto coContext::AsyncWaiter::getSubmissionQueueEntry() const noexcept -> SubmissionQueueEntry {
    return this->submissionQueueEntry;
}

auto coContext::AsyncWaiter::getTaskIdentity() const noexcept -> std::uint64_t { return this->taskIdentity; }

auto coContext::AsyncWaiter::getFirstTimeSpecification() const noexcept -> __kernel_timespec {
    return this->timeSpecifications.first;
}

auto coContext::AsyncWaiter::getFirstTimeSpecification() noexcept -> __kernel_timespec & {
    return this->timeSpecifications.first;
}

auto coContext::AsyncWaiter::setFirstTimeSpecification(const std::chrono::seconds seconds,
                                                       const std::chrono::nanoseconds nanoseconds) -> void {
    this->timeSpecifications.first.tv_sec = seconds.count();
    this->timeSpecifications.first.tv_nsec = nanoseconds.count();
}

auto coContext::AsyncWaiter::getSecondTimeSpecification() const noexcept -> __kernel_timespec {
    return this->timeSpecifications.second;
}

auto coContext::AsyncWaiter::getSecondTimeSpecification() noexcept -> __kernel_timespec & {
    return this->timeSpecifications.second;
}

auto coContext::AsyncWaiter::setSecondTimeSpecification(const std::chrono::seconds seconds,
                                                        const std::chrono::nanoseconds nanoseconds) -> void {
    this->timeSpecifications.second.tv_sec = seconds.count();
    this->timeSpecifications.second.tv_nsec = nanoseconds.count();
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
    const __kernel_timespec lhsFirstTimeSpecification{lhs.getFirstTimeSpecification()},
        lhsSecondTimeSpecification{lhs.getSecondTimeSpecification()},
        rhsFirstTimeSpecification{rhs.getFirstTimeSpecification()},
        rhsSecondTimeSpecification{rhs.getSecondTimeSpecification()};

    return lhs.getTasks() == rhs.getTasks() && lhs.getSubmissionQueueEntry() == rhs.getSubmissionQueueEntry() &&
           lhs.getTaskIdentity() == rhs.getTaskIdentity() &&
           lhsFirstTimeSpecification.tv_sec == rhsFirstTimeSpecification.tv_sec &&
           lhsFirstTimeSpecification.tv_nsec == rhsFirstTimeSpecification.tv_nsec &&
           lhsSecondTimeSpecification.tv_sec == rhsSecondTimeSpecification.tv_sec &&
           lhsSecondTimeSpecification.tv_nsec == rhsSecondTimeSpecification.tv_nsec;
}


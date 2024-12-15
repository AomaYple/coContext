#include "coContext/coroutine/AsyncWaiter.hpp"

#include "coContext/coroutine/BasePromise.hpp"

coContext::AsyncWaiter::AsyncWaiter(const SubmissionQueueEntry submissionQueueEntry) noexcept :
    submissionQueueEntry{submissionQueueEntry} {}

auto coContext::AsyncWaiter::swap(AsyncWaiter &other) noexcept -> void {
    std::swap(this->submissionQueueEntry, other.submissionQueueEntry);
    std::swap(this->result, other.result);
    std::swap(this->timeSpecifications, other.timeSpecifications);
}

auto coContext::AsyncWaiter::getSubmissionQueueEntry() const noexcept -> SubmissionQueueEntry {
    return this->submissionQueueEntry;
}

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

auto coContext::AsyncWaiter::await_suspend(const std::coroutine_handle<> genericCoroutineHandle) -> void {
    const auto coroutineHandle{Coroutine::Handle::from_address(genericCoroutineHandle.address())};

    this->submissionQueueEntry.setUserData(std::hash<Coroutine::Handle>{}(coroutineHandle));
    this->result = coroutineHandle.promise().getResult();
}

auto coContext::AsyncWaiter::await_resume() const noexcept -> std::int32_t { return *this->result; }

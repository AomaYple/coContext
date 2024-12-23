#include "coContext/coroutine/AsyncWaiter.hpp"

#include "coContext/coroutine/BasePromise.hpp"

coContext::internal::AsyncWaiter::AsyncWaiter(const SubmissionQueueEntry submissionQueueEntry) noexcept :
    submissionQueueEntry{submissionQueueEntry} {}

auto coContext::internal::AsyncWaiter::swap(AsyncWaiter &other) noexcept -> void {
    std::swap(this->submissionQueueEntry, other.submissionQueueEntry);
    std::swap(this->coroutineHandle, other.coroutineHandle);
    std::swap(this->timeSpecification, other.timeSpecification);
}

auto coContext::internal::AsyncWaiter::getSubmissionQueueEntry() const noexcept -> SubmissionQueueEntry {
    return this->submissionQueueEntry;
}

auto coContext::internal::AsyncWaiter::getTimeSpecification() const noexcept
    -> const std::unique_ptr<__kernel_timespec> & {
    return this->timeSpecification;
}

auto coContext::internal::AsyncWaiter::setTimeSpecification(
    std::unique_ptr<__kernel_timespec> timeSpecification) noexcept -> void {
    this->timeSpecification = std::move(timeSpecification);
}

auto coContext::internal::AsyncWaiter::await_ready() const noexcept -> bool { return {}; }

auto coContext::internal::AsyncWaiter::await_suspend(const std::coroutine_handle<> genericCoroutineHandle) noexcept
    -> void {
    if (this->coroutineHandle == genericCoroutineHandle) return;

    this->coroutineHandle = Coroutine::Handle::from_address(genericCoroutineHandle.address());
    this->submissionQueueEntry.setUserData(std::hash<Coroutine::Handle>{}(this->coroutineHandle));
}

auto coContext::internal::AsyncWaiter::await_resume() const -> std::int32_t {
    return this->coroutineHandle.promise().getResult();
}

auto coContext::internal::AsyncWaiter::getAsyncWaitResumeFlags() const -> std::uint32_t {
    return this->coroutineHandle.promise().getFlags();
}

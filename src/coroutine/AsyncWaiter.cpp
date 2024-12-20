#include "coContext/coroutine/AsyncWaiter.hpp"

#include "coContext/coroutine/BasePromise.hpp"

coContext::AsyncWaiter::AsyncWaiter(const SubmissionQueueEntry submissionQueueEntry) noexcept :
    submissionQueueEntry{submissionQueueEntry} {}

auto coContext::AsyncWaiter::swap(AsyncWaiter &other) noexcept -> void {
    std::swap(this->submissionQueueEntry, other.submissionQueueEntry);
    std::swap(this->coroutineHandle, other.coroutineHandle);
    std::swap(this->timeSpecifications, other.timeSpecifications);
}

auto coContext::AsyncWaiter::getSubmissionQueueEntry() const noexcept -> SubmissionQueueEntry {
    return this->submissionQueueEntry;
}

auto coContext::AsyncWaiter::getTimeSpecifications() noexcept -> TimeSpecification & {
    return this->timeSpecifications;
}

auto coContext::AsyncWaiter::await_ready() const noexcept -> bool { return {}; }

auto coContext::AsyncWaiter::await_suspend(const std::coroutine_handle<> genericCoroutineHandle) noexcept -> void {
    this->coroutineHandle = Coroutine::Handle::from_address(genericCoroutineHandle.address());
    this->submissionQueueEntry.setUserData(std::hash<Coroutine::Handle>{}(this->coroutineHandle));
}

auto coContext::AsyncWaiter::await_resume() const -> std::int32_t {
    return this->coroutineHandle.promise().getResult();
}

auto coContext::AsyncWaiter::getAsyncWaitResumeFlags() const -> std::uint32_t {
    return this->coroutineHandle.promise().getFlags();
}

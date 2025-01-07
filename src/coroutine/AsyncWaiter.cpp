#include "coContext/coroutine/AsyncWaiter.hpp"

#include "coContext/coroutine/BasePromise.hpp"

coContext::internal::AsyncWaiter::AsyncWaiter(const Submission submission) noexcept : submission{submission} {}

auto coContext::internal::AsyncWaiter::swap(AsyncWaiter &other) noexcept -> void {
    std::swap(this->submission, other.submission);
    std::swap(this->coroutineHandle, other.coroutineHandle);
    std::swap(this->timeSpecification, other.timeSpecification);
}

auto coContext::internal::AsyncWaiter::getSubmission() const noexcept -> Submission { return this->submission; }

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
    this->submission.setUserData(std::hash<Coroutine::Handle>{}(this->coroutineHandle));
}

auto coContext::internal::AsyncWaiter::await_resume() const -> std::int32_t {
    return this->coroutineHandle.promise().getResult();
}

auto coContext::internal::AsyncWaiter::getResumeFlags() const -> std::uint32_t {
    return this->coroutineHandle.promise().getFlags();
}

#include "coContext/coroutine/BaseTask.hpp"

#include "coContext/coroutine/BasePromise.hpp"

auto coContext::internal::BaseTask::swap(BaseTask &other) noexcept -> void {
    std::swap(this->coroutine, other.coroutine);
}

auto coContext::internal::BaseTask::getCoroutine() noexcept -> Coroutine & { return this->coroutine; }

auto coContext::internal::BaseTask::await_ready() const noexcept -> bool { return {}; }

auto coContext::internal::BaseTask::await_suspend(const std::coroutine_handle<> genericCoroutineHandle) -> void {
    const auto parentCoroutineHandle{Coroutine::Handle::from_address(genericCoroutineHandle.address())};

    this->coroutine.getPromise().setParentCoroutineId(std::hash<Coroutine::Handle>{}(parentCoroutineHandle));
    parentCoroutineHandle.promise().setChildCoroutine(std::move(this->coroutine));
}

coContext::internal::BaseTask::BaseTask(Coroutine coroutine) noexcept : coroutine{std::move(coroutine)} {}

#include "coContext/coroutine/BaseTask.hpp"

#include "coContext/coroutine/BasePromise.hpp"

auto coContext::BaseTask::swap(BaseTask &other) noexcept -> void { std::swap(this->coroutine, other.coroutine); }

auto coContext::BaseTask::getCoroutine() noexcept -> Coroutine & { return this->coroutine; }

auto coContext::BaseTask::await_ready() const noexcept -> bool { return {}; }

auto coContext::BaseTask::await_suspend(const std::coroutine_handle<> genericCoroutineHandle) -> void {
    const auto parentCoroutineHandle{Coroutine::Handle::from_address(genericCoroutineHandle.address())};

    this->coroutine.promise().setParentCoroutineIdentity(std::hash<Coroutine::Handle>{}(parentCoroutineHandle));
    parentCoroutineHandle.promise().setChildCoroutine(std::move(this->coroutine));
}

coContext::BaseTask::BaseTask(Coroutine &&coroutine) noexcept : coroutine{std::move(coroutine)} {}

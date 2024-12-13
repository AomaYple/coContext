#include "coContext/coroutine/BaseTask.hpp"

#include "coContext/coroutine/BasePromise.hpp"

coContext::BaseTask::BaseTask(const Coroutine coroutine) noexcept : coroutine{coroutine} {}

auto coContext::BaseTask::getCoroutine() const noexcept -> Coroutine { return this->coroutine; }

auto coContext::BaseTask::await_ready() const noexcept -> bool { return {}; }

auto coContext::BaseTask::await_suspend(const std::coroutine_handle<> coroutine) const -> void {
    const Coroutine parentCoroutine{Coroutine::from_address(coroutine.address())};

    this->coroutine.promise().setParentCoroutine(parentCoroutine);
    parentCoroutine.promise().setChildCoroutine(this->coroutine);
}

auto coContext::operator==(const BaseTask lhs, const BaseTask rhs) noexcept -> bool {
    return lhs.getCoroutine() == rhs.getCoroutine();
}


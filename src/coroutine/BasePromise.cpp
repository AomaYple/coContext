#include "coContext/coroutine/BasePromise.hpp"

auto coContext::BasePromise::initial_suspend() const noexcept -> std::suspend_always { return {}; }

auto coContext::BasePromise::final_suspend() const noexcept -> std::suspend_always { return {}; }

auto coContext::BasePromise::unhandled_exception() const -> void { throw; }

auto coContext::BasePromise::getResult() const noexcept -> std::int32_t { return this->result; }

auto coContext::BasePromise::setResult(const std::int32_t result) noexcept -> void { this->result = result; }

auto coContext::BasePromise::getParentCoroutine() const noexcept -> Coroutine { return this->parentCoroutine; }

auto coContext::BasePromise::setParentCoroutine(const Coroutine parentCoroutine) noexcept -> void {
    this->parentCoroutine = parentCoroutine;
}

auto coContext::BasePromise::getChildCoroutine() const noexcept -> Coroutine { return this->childCoroutine; }

auto coContext::BasePromise::setChildCoroutine(const Coroutine childCoroutine) noexcept -> void {
    this->childCoroutine = childCoroutine;
}

auto coContext::operator==(const BasePromise &lhs, const BasePromise &rhs) noexcept -> bool {
    return lhs.getResult() == rhs.getResult() && lhs.getParentCoroutine() == rhs.getParentCoroutine() &&
           lhs.getChildCoroutine() == rhs.getChildCoroutine();
}

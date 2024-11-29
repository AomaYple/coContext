#include "coContext/coroutine/Promise.hpp"

#include "coContext/coroutine/Task.hpp"

auto coContext::Promise::get_return_object() -> Task {
    return Task{std::coroutine_handle<Promise>::from_promise(*this)};
}

auto coContext::Promise::initial_suspend() const noexcept -> std::suspend_always { return {}; }

auto coContext::Promise::final_suspend() const noexcept -> std::suspend_always { return {}; }

auto coContext::Promise::unhandled_exception() const -> void { throw; }

auto coContext::Promise::return_void() const noexcept -> void {}

auto coContext::Promise::setResult(const std::int32_t result) noexcept -> void { this->result = result; }

auto coContext::Promise::getResult() const noexcept -> std::int32_t { return this->result; }

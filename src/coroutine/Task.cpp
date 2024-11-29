#include "coContext/coroutine/Task.hpp"

#include <utility>

auto coContext::Task::Promise::get_return_object() -> Task {
    return Task{std::coroutine_handle<Promise>::from_promise(*this)};
}

auto coContext::Task::Promise::initial_suspend() const noexcept -> std::suspend_always { return {}; }

auto coContext::Task::Promise::final_suspend() const noexcept -> std::suspend_always { return {}; }

auto coContext::Task::Promise::unhandled_exception() const -> void { throw; }

auto coContext::Task::Promise::return_void() const noexcept -> void {}

auto coContext::Task::Promise::setResult(const std::int32_t result) noexcept -> void { this->result = result; }

auto coContext::Task::Promise::getResult() const noexcept -> std::int32_t { return this->result; }

coContext::Task::Task(Task &&other) noexcept : handle{std::exchange(other.handle, nullptr)} {}

auto coContext::Task::operator=(Task &&other) noexcept -> Task & {
    if (this == &other) return *this;

    this->destroy();

    this->handle = std::exchange(other.handle, nullptr);

    return *this;
}

coContext::Task::~Task() { this->destroy(); }

auto coContext::Task::swap(Task &other) noexcept -> void { std::swap(this->handle, other.handle); }

auto coContext::Task::getHash() const noexcept -> std::size_t {
    return std::hash<std::coroutine_handle<Promise>>{}(this->handle);
}

auto coContext::Task::operator()(const std::int32_t result) const -> void {
    this->handle.promise().setResult(result);
    this->handle();
}

auto coContext::Task::done() const noexcept -> bool { return this->handle.done(); }

coContext::Task::Task(const std::coroutine_handle<Promise> handle) noexcept : handle{handle} {}

auto coContext::Task::destroy() const -> void {
    if (static_cast<bool>(this->handle)) this->handle.destroy();
}

auto coContext::operator==(const Task::Promise &lhs, const Task::Promise &rhs) noexcept -> bool {
    return lhs.getResult() == rhs.getResult();
}

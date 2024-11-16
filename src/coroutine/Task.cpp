#include "coContext/coroutine/Task.hpp"

#include <utility>

auto coContext::Task::promise_type::get_return_object() -> Task {
    return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
}

auto coContext::Task::promise_type::unhandled_exception() const -> void { throw; }

auto coContext::Task::promise_type::setSubmission(const Submission &submission) noexcept -> void {
    this->submission = submission;
}

auto coContext::Task::promise_type::getSubmission() const noexcept -> const Submission & { return this->submission; }

auto coContext::Task::promise_type::setResult(const int result) noexcept -> void { this->result = result; }

auto coContext::Task::promise_type::getResult() const noexcept -> int { return this->result; }

coContext::Task::Task(const std::coroutine_handle<promise_type> handle) noexcept : handle{handle} {}

coContext::Task::Task(Task &&other) noexcept : handle{std::exchange(other.handle, nullptr)} {}

auto coContext::Task::operator=(Task &&other) noexcept -> Task & {
    if (this == &other) return *this;

    this->destroy();

    this->handle = std::exchange(other.handle, nullptr);

    return *this;
}

coContext::Task::~Task() { this->destroy(); }

auto coContext::Task::getSubmission() const -> const Submission & { return this->handle.promise().getSubmission(); }

auto coContext::Task::operator()(const int result) const -> void {
    this->handle.promise().setResult(result);
    this->handle();
}

auto coContext::Task::done() const noexcept -> bool { return this->handle.done(); }

auto coContext::Task::destroy() const -> void {
    if (static_cast<bool>(this->handle)) this->handle.destroy();
}

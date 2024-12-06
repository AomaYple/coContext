#include "coContext/coroutine/AsyncWaiter.hpp"

#include "coContext/coroutine/GenericTask.hpp"

#include <liburing.h>

coContext::AsyncWaiter::AsyncWaiter(Tasks tasks, io_uring_sqe *const submissionQueueEntry) noexcept :
    tasks{std::move(tasks)}, submissionQueueEntry{submissionQueueEntry} {}

auto coContext::AsyncWaiter::swap(AsyncWaiter &other) noexcept -> void {
    std::swap(this->tasks, other.tasks);
    std::swap(this->submissionQueueEntry, other.submissionQueueEntry);
    std::swap(this->taskIdentity, other.taskIdentity);
}

auto coContext::AsyncWaiter::await_ready() const noexcept -> bool { return {}; }

auto coContext::AsyncWaiter::await_suspend(const std::coroutine_handle<> handle) noexcept -> void {
    this->taskIdentity = std::hash<std::coroutine_handle<>>{}(handle);
    io_uring_sqe_set_data64(this->submissionQueueEntry, this->taskIdentity);
}

auto coContext::AsyncWaiter::await_resume() const -> std::int32_t {
    return this->tasks->at(this->taskIdentity).getResult();
}

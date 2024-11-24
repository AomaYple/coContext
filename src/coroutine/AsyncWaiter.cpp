#include "coContext/coroutine/AsyncWaiter.hpp"

#include <liburing.h>

coContext::AsyncWaiter::AsyncWaiter(io_uring_sqe *const submissionQueueEntry) noexcept :
    submissionQueueEntry{submissionQueueEntry} {}

auto coContext::AsyncWaiter::swap(AsyncWaiter &other) noexcept -> void {
    std::swap(this->submissionQueueEntry, other.submissionQueueEntry);
    std::swap(this->handle, other.handle);
}

auto coContext::AsyncWaiter::await_ready() const noexcept -> bool { return false; }

auto coContext::AsyncWaiter::await_suspend(const std::coroutine_handle<Task::promise_type> handle) noexcept -> void {
    this->handle = handle;
    io_uring_sqe_set_data64(this->submissionQueueEntry,
                            std::hash<std::coroutine_handle<Task::promise_type>>{}(this->handle));
}

auto coContext::AsyncWaiter::await_resume() const -> std::int32_t { return this->handle.promise().getResult(); }

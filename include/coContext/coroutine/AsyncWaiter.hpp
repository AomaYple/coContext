#pragma once

#include "Task.hpp"

#include <liburing/io_uring.h>

namespace coContext {
    class AsyncWaiter {
    public:
        explicit AsyncWaiter(io_uring_sqe *submissionQueueEntry) noexcept;

        AsyncWaiter(const AsyncWaiter &) = delete;

        auto operator=(const AsyncWaiter &) -> AsyncWaiter & = delete;

        AsyncWaiter(AsyncWaiter &&) noexcept = default;

        auto operator=(AsyncWaiter &&) noexcept -> AsyncWaiter & = default;

        ~AsyncWaiter() = default;

        [[nodiscard]] auto await_ready() const noexcept -> bool;

        auto await_suspend(std::coroutine_handle<Task::promise_type> handle) -> void;

        [[nodiscard]] auto await_resume() const -> std::int32_t;

    private:
        io_uring_sqe *submissionQueueEntry;
        std::coroutine_handle<Task::promise_type> handle;
    };
}    // namespace coContext

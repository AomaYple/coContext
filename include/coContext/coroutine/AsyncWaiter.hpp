#pragma once

#include <coroutine>
#include <liburing/io_uring.h>
#include <memory>
#include <unordered_map>

namespace coContext {
    class GenericTask;

    class AsyncWaiter {
        using Tasks = std::shared_ptr<const std::unordered_map<std::size_t, GenericTask>>;

    public:
        AsyncWaiter(Tasks tasks, io_uring_sqe *submissionQueueEntry) noexcept;

        AsyncWaiter(const AsyncWaiter &) = delete;

        auto operator=(const AsyncWaiter &) -> AsyncWaiter & = delete;

        AsyncWaiter(AsyncWaiter &&) noexcept = default;

        auto operator=(AsyncWaiter &&) noexcept -> AsyncWaiter & = default;

        ~AsyncWaiter() = default;

        auto swap(AsyncWaiter &other) noexcept -> void;

        [[nodiscard]] auto await_ready() const noexcept -> bool;

        auto await_suspend(std::coroutine_handle<> handle) noexcept -> void;

        [[nodiscard]] auto await_resume() const -> std::int32_t;

    private:
        Tasks tasks;
        io_uring_sqe *submissionQueueEntry;
        std::size_t coroutineHash;
    };
}    // namespace coContext

template<>
constexpr auto std::swap(coContext::AsyncWaiter &lhs, coContext::AsyncWaiter &rhs) noexcept -> void {
    lhs.swap(rhs);
}

#pragma once

#include <coroutine>
#include <liburing.h>

namespace coContext {
    class Task {
    public:
        class promise_type {
        public:
            [[nodiscard]] auto get_return_object() -> Task;

            [[nodiscard]] auto initial_suspend() const noexcept -> std::suspend_never;

            [[nodiscard]] auto final_suspend() const noexcept -> std::suspend_always;

            auto unhandled_exception() const -> void;

            auto setSubmissionQueueEntry(io_uring_sqe *submissionQueueEntry) noexcept -> void;

            [[nodiscard]] auto getSubmissionQueueEntry() const noexcept -> io_uring_sqe *;

            auto setResult(std::int32_t result) noexcept -> void;

            [[nodiscard]] auto getResult() const noexcept -> std::int32_t;

        private:
            io_uring_sqe *submissionQueueEntry;
            std::int32_t result;
        };

        Task(const Task &) = delete;

        auto operator=(const Task &) -> Task & = delete;

        Task(Task &&) noexcept;

        auto operator=(Task &&) noexcept -> Task &;

        ~Task();

        [[nodiscard]] auto getSubmissionQueueEntry() const -> io_uring_sqe *;

        [[nodiscard]] auto getHash() const noexcept -> std::uint64_t;

        auto operator()(std::int32_t result) const -> void;

        [[nodiscard]] auto done() const noexcept -> bool;

    private:
        explicit Task(std::coroutine_handle<promise_type> handle) noexcept;

        auto destroy() const -> void;

        std::coroutine_handle<promise_type> handle;
    };
}    // namespace coContext

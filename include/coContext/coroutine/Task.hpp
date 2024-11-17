#pragma once

#include <coroutine>

namespace coContext {
    class Task {
    public:
        class promise_type {
        public:
            [[nodiscard]] auto get_return_object() -> Task;

            [[nodiscard]] auto initial_suspend() const noexcept -> std::suspend_never;

            [[nodiscard]] auto final_suspend() const noexcept -> std::suspend_always;

            auto unhandled_exception() const -> void;

            auto setResult(int result) noexcept -> void;

            [[nodiscard]] auto getResult() const noexcept -> int;

        private:
            int result;
        };

        explicit Task(std::coroutine_handle<promise_type> handle) noexcept;

        Task(const Task &) = delete;

        auto operator=(const Task &) -> Task & = delete;

        Task(Task &&) noexcept;

        auto operator=(Task &&) noexcept -> Task &;

        ~Task();

        [[nodiscard]] auto getHash() const noexcept -> unsigned long;

        auto operator()(int result) const -> void;

        [[nodiscard]] auto done() const noexcept -> bool;

    private:
        auto destroy() const -> void;

        std::coroutine_handle<promise_type> handle;
    };
}    // namespace coContext

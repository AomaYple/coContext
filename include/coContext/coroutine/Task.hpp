#pragma once

#include <coroutine>
#include <cstdint>

namespace coContext {
    class Task {
    public:
        class Promise {
        public:
            [[nodiscard]] auto get_return_object() -> Task;

            [[nodiscard]] auto initial_suspend() const noexcept -> std::suspend_always;

            [[nodiscard]] auto final_suspend() const noexcept -> std::suspend_always;

            auto unhandled_exception() const -> void;

            auto return_void() const noexcept -> void;

            auto setResult(std::int32_t result) noexcept -> void;

            [[nodiscard]] auto getResult() const noexcept -> std::int32_t;

        private:
            std::int32_t result;
        };

        using promise_type = Promise;

        Task(const Task &) = delete;

        auto operator=(const Task &) -> Task & = delete;

        Task(Task &&) noexcept;

        auto operator=(Task &&) noexcept -> Task &;

        ~Task();

        auto swap(Task &other) noexcept -> void;

        [[nodiscard]] auto getHash() const noexcept -> std::size_t;

        auto operator()(std::int32_t result) const -> void;

        [[nodiscard]] auto done() const noexcept -> bool;

    private:
        explicit Task(std::coroutine_handle<Promise> handle) noexcept;

        auto destroy() const -> void;

        std::coroutine_handle<Promise> handle;
    };

    [[nodiscard]] auto operator==(const Task::Promise &lhs, const Task::Promise &rhs) noexcept -> bool;
}    // namespace coContext

template<>
constexpr auto std::swap(coContext::Task &lhs, coContext::Task &rhs) noexcept -> void {
    lhs.swap(rhs);
}

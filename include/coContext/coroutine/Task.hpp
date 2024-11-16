#pragma once

#include "coContext/ring/Submission.hpp"

#include <coroutine>

namespace coContext {
    class Task {
    public:
        class promise_type {
        public:
            [[nodiscard]] auto get_return_object() -> Task;

            [[nodiscard]] constexpr auto initial_suspend() const noexcept { return std::suspend_never{}; }

            [[nodiscard]] constexpr auto final_suspend() const noexcept { return std::suspend_always{}; }

            auto unhandled_exception() const -> void;

            auto setSubmission(const Submission &submission) noexcept -> void;

            [[nodiscard]] auto getSubmission() const noexcept -> const Submission &;

            auto setResult(int result) noexcept -> void;

            [[nodiscard]] auto getResult() const noexcept -> int;

        private:
            Submission submission;
            int result;
        };

        explicit Task(std::coroutine_handle<promise_type> handle) noexcept;

        Task(const Task &) = delete;

        Task(Task &&) noexcept;

        auto operator=(const Task &) -> Task & = delete;

        auto operator=(Task &&) noexcept -> Task &;

        ~Task();

        [[nodiscard]] auto getSubmission() const -> const Submission &;

        auto operator()(int result) const -> void;

        [[nodiscard]] auto done() const noexcept -> bool;

    private:
        auto destroy() const -> void;

        std::coroutine_handle<promise_type> handle;
    };
}    // namespace coContext

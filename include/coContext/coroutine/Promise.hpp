#pragma once

#include <coroutine>
#include <cstdint>

namespace coContext {
    class Task;

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
}    // namespace coContext

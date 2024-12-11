#pragma once

#include <coroutine>
#include <cstdint>

namespace coContext {
    class BasePromise {
    public:
        [[nodiscard]] auto initial_suspend() const noexcept -> std::suspend_always;

        [[nodiscard]] auto final_suspend() const noexcept -> std::suspend_always;

        auto unhandled_exception() const -> void;

        [[nodiscard]] auto getResult() const noexcept -> std::int32_t;

        auto setResult(std::int32_t result) noexcept -> void;

    private:
        std::int32_t result{};
    };

    [[nodiscard]] auto operator==(BasePromise lhs, BasePromise rhs) noexcept -> bool;
}    // namespace coContext

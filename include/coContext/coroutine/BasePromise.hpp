#pragma once

#include <coroutine>
#include <cstdint>

namespace coContext {
    class BasePromise {
        using Coroutine = std::coroutine_handle<BasePromise>;

    public:
        [[nodiscard]] auto initial_suspend() const noexcept -> std::suspend_always;

        [[nodiscard]] auto final_suspend() const noexcept -> std::suspend_always;

        auto unhandled_exception() const -> void;

        [[nodiscard]] auto getResult() const noexcept -> std::int32_t;

        auto setResult(std::int32_t result) noexcept -> void;

        [[nodiscard]] auto getParentCoroutine() const noexcept -> Coroutine;

        auto setParentCoroutine(Coroutine parentCoroutine) noexcept -> void;

        [[nodiscard]] auto getChildCoroutine() const noexcept -> Coroutine;

        auto setChildCoroutine(Coroutine childCoroutine) noexcept -> void;

    private:
        std::int32_t result{};
        Coroutine parentCoroutine{Coroutine::from_address(std::noop_coroutine().address())},
            childCoroutine{Coroutine::from_address(std::noop_coroutine().address())};
    };

    [[nodiscard]] auto operator==(const BasePromise &lhs, const BasePromise &rhs) noexcept -> bool;
}    // namespace coContext

#pragma once

#include <coroutine>

namespace coContext {
    class BasePromise;

    class BaseTask {
    public:
        using Coroutine = std::coroutine_handle<BasePromise>;

        explicit BaseTask(Coroutine coroutine = Coroutine::from_address(std::noop_coroutine().address())) noexcept;

        [[nodiscard]] auto getCoroutine() const noexcept -> Coroutine;

        [[nodiscard]] auto await_ready() const noexcept -> bool;

        auto await_suspend(std::coroutine_handle<> coroutine) const -> void;

    private:
        Coroutine coroutine;
    };

    [[nodiscard]] auto operator==(BaseTask lhs, BaseTask rhs) noexcept -> bool;
}    // namespace coContext

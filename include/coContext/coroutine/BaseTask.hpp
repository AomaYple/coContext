#pragma once

#include "Coroutine.hpp"

#include <memory>

namespace coContext::internal {
    class BaseTask {
    public:
        BaseTask(const BaseTask &) = delete;

        auto operator=(const BaseTask &) -> BaseTask & = delete;

        BaseTask(BaseTask &&) noexcept = default;

        auto operator=(BaseTask &&) noexcept -> BaseTask & = default;

        ~BaseTask() = default;

        auto swap(BaseTask &other) noexcept -> void;

        [[nodiscard]] auto getCoroutine() noexcept -> Coroutine &;

        auto throwException() const -> void;

        [[nodiscard]] auto await_ready() const noexcept -> bool;

        auto await_suspend(std::coroutine_handle<> genericCoroutineHandle) -> void;

    protected:
        BaseTask(Coroutine coroutine, std::shared_ptr<std::exception_ptr> exception) noexcept;

    private:
        Coroutine coroutine;
        std::shared_ptr<std::exception_ptr> exception;
    };
}    // namespace coContext::internal

template<>
constexpr auto std::swap(coContext::internal::BaseTask &lhs, coContext::internal::BaseTask &rhs) noexcept -> void {
    lhs.swap(rhs);
}

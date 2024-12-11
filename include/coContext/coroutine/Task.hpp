#pragma once

#include "BasePromise.hpp"

#include <future>

namespace coContext {
    template<typename T = void>
        requires std::movable<T> || std::is_void_v<T>
    class Task {
    public:
        class Promise;

        using promise_type = Promise;
        using Coroutine = std::coroutine_handle<Promise>;

        class Promise : public BasePromise {
        public:
            constexpr Promise() = default;

            Promise(const Promise &) = delete;

            auto operator=(const Promise &) -> Promise & = delete;

            constexpr Promise(Promise &&) noexcept = default;

            constexpr auto operator=(Promise &&) noexcept -> Promise & = default;

            constexpr ~Promise() = default;

            constexpr auto swap(Promise &other) noexcept { std::swap(this->returnValue, other.returnValue); }

            [[nodiscard]] constexpr auto get_return_object() { return Task{Coroutine::from_promise(*this)}; }

            constexpr auto return_value(T &&returnValue) { this->returnValue.set_value(std::move(returnValue)); }

            constexpr auto return_value(const T &returnValue)
                requires std::copyable<T>
            {
                this->returnValue.set_value(returnValue);
            }

            [[nodiscard]] constexpr auto getReturnValue() { return this->returnValue.get_future(); }

        private:
            std::promise<T> returnValue;
        };

        explicit constexpr Task(const Coroutine coroutine) :
            coroutine{coroutine}, returnValue{coroutine.promise().getReturnValue()} {}

        Task(const Task &) = delete;

        auto operator=(const Task &) -> Task & = delete;

        constexpr Task(Task &&other) noexcept = default;

        constexpr auto operator=(Task &&other) noexcept -> Task & = default;

        constexpr ~Task() = default;

        constexpr auto swap(Task &other) noexcept {
            std::swap(this->coroutine, other.coroutine);
            std::swap(this->returnValue, other.returnValue);
        }

        [[nodiscard]] constexpr auto getCoroutine() const noexcept -> Coroutine { return this->coroutine; }

        [[nodiscard]] constexpr auto getReturnValue() noexcept -> std::future<T> & { return this->returnValue; }

    private:
        Coroutine coroutine;
        std::future<T> returnValue;
    };

    template<>
    class Task<> {
    public:
        class Promise;

        using promise_type = Promise;
        using Coroutine = std::coroutine_handle<Promise>;

        class Promise : public BasePromise {
        public:
            [[nodiscard]] constexpr auto get_return_object() { return Task{Coroutine::from_promise(*this)}; }

            constexpr auto return_void() const noexcept {}
        };

        explicit constexpr Task(const Coroutine coroutine = Coroutine::from_address(
                                    std::noop_coroutine().address())) noexcept : coroutine{coroutine} {}

        constexpr Task(const Task &) noexcept = default;

        constexpr auto operator=(const Task &) noexcept -> Task & = default;

        constexpr Task(Task &&) noexcept = default;

        constexpr auto operator=(Task &&) noexcept -> Task & = default;

        constexpr ~Task() = default;

        [[nodiscard]] constexpr auto getCoroutine() const noexcept -> Coroutine { return this->coroutine; }

    private:
        Coroutine coroutine;
    };

    [[nodiscard]] auto operator==(Task<>::Promise lhs, Task<>::Promise rhs) noexcept -> bool;

    [[nodiscard]] auto operator==(Task<> lhs, Task<> rhs) noexcept -> bool;
}    // namespace coContext

namespace std {
    template<std::movable T>
    constexpr auto swap(typename coContext::Task<T>::Promise &lhs, typename coContext::Task<T>::Promise &rhs) noexcept
        -> void {
        lhs.swap(rhs);
    }

    template<std::movable T>
    constexpr auto swap(coContext::Task<T> &lhs, coContext::Task<T> &rhs) noexcept -> void {
        lhs.swap(rhs);
    }
}    // namespace std

#pragma once

#include "Coroutine.hpp"

#include <future>

namespace coContext {
    template<typename T>
    concept TaskReturnValueType = std::movable<T> || std::is_void_v<T>;

    template<TaskReturnValueType T = void>
    class Task {
    public:
        class Promise {
        public:
            constexpr Promise() = default;

            Promise(const Promise &) = delete;

            auto operator=(const Promise &) -> Promise & = delete;

            constexpr Promise(Promise &&) noexcept = default;

            constexpr auto operator=(Promise &&) noexcept -> Promise & = default;

            constexpr ~Promise() = default;

            constexpr auto swap(Promise &other) noexcept { std::swap(this->returnValue, other.returnValue); }

            [[nodiscard]] constexpr auto get_return_object() {
                return Task{std::coroutine_handle<Promise>::from_promise(*this)};
            }

            [[nodiscard]] constexpr auto initial_suspend() const noexcept { return std::suspend_always{}; }

            [[nodiscard]] constexpr auto final_suspend() const noexcept { return std::suspend_always{}; }

            constexpr auto unhandled_exception() const { throw; }

            constexpr auto return_value(T &&returnValue) { this->returnValue.set_value(std::move(returnValue)); }

            [[nodiscard]] constexpr auto getReturnValue() { return this->returnValue.get_future(); }

        private:
            std::promise<T> returnValue;
        };

        using promise_type = Promise;

        explicit constexpr Task(const std::coroutine_handle<Promise> handle) :
            coroutine{handle}, returnValue{handle.promise().getReturnValue()} {}

        Task(const Task &) = delete;

        auto operator=(const Task &) -> Task & = delete;

        constexpr Task(Task &&other) noexcept = default;

        constexpr auto operator=(Task &&other) noexcept -> Task & = default;

        constexpr ~Task() = default;

        constexpr auto swap(Task &other) noexcept {
            std::swap(this->coroutine, other.coroutine);
            std::swap(this->returnValue, other.returnValue);
        }

        [[nodiscard]] constexpr auto getCoroutine() noexcept -> Coroutine & { return this->coroutine; }

        [[nodiscard]] constexpr auto getReturnValue() noexcept -> std::future<T> & { return this->returnValue; }

    private:
        Coroutine coroutine;
        std::future<T> returnValue;
    };

    template<>
    class Task<> {
    public:
        class Promise {
        public:
            [[nodiscard]] constexpr auto get_return_object() {
                return Task{std::coroutine_handle<Promise>::from_promise(*this)};
            }

            [[nodiscard]] constexpr auto initial_suspend() const noexcept { return std::suspend_always{}; }

            [[nodiscard]] constexpr auto final_suspend() const noexcept { return std::suspend_always{}; }

            constexpr auto unhandled_exception() const { throw; }

            constexpr auto return_void() const noexcept {}
        };

        using promise_type = Promise;

        explicit constexpr Task(const std::coroutine_handle<Promise> handle) noexcept : coroutine{handle} {}

        Task(const Task &) = delete;

        auto operator=(const Task &) -> Task & = delete;

        Task(Task &&) noexcept = default;

        auto operator=(Task &&) noexcept -> Task & = default;

        ~Task() = default;

        constexpr auto swap(Task &other) noexcept { std::swap(this->coroutine, other.coroutine); }

        [[nodiscard]] constexpr auto getCoroutine() noexcept -> Coroutine & { return this->coroutine; }

    private:
        Coroutine coroutine;
    };
}    // namespace coContext

namespace std {
    template<coContext::TaskReturnValueType T>
    constexpr auto swap(coContext::Task<T> &lhs, coContext::Task<T> &rhs) noexcept -> void {
        lhs.swap(rhs);
    }

    template<coContext::TaskReturnValueType T>
    constexpr auto swap(typename coContext::Task<T>::Promise &lhs, typename coContext::Task<T>::Promise &rhs) noexcept
        -> void {
        lhs.swap(rhs);
    }
}    // namespace std

template<>
constexpr auto std::swap(coContext::Task<> &lhs, coContext::Task<> &rhs) noexcept -> void {
    lhs.swap(rhs);
}

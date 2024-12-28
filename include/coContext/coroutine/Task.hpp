#pragma once

#include "BasePromise.hpp"
#include "BaseTask.hpp"

#include <future>

namespace coContext {
    namespace internal {
        template<typename T>
        concept Returnable = std::movable<T> || std::is_lvalue_reference_v<T> || std::is_void_v<T>;
    }    // namespace internal

    template<internal::Returnable T = void>
    class Task : public internal::BaseTask {
        class Promise;

        using CoroutineHandle = std::coroutine_handle<Promise>;

        class Promise : public internal::BasePromise {
        public:
            constexpr Promise() = default;

            Promise(const Promise &) = delete;

            auto operator=(const Promise &) -> Promise & = delete;

            constexpr Promise(Promise &&) noexcept = default;

            constexpr auto operator=(Promise &&) noexcept -> Promise & = default;

            constexpr ~Promise() = default;

            constexpr auto swap(Promise &other) noexcept {
                std::swap(static_cast<BasePromise &>(*this), static_cast<BasePromise &>(other));
                std::swap(this->returnValue, other.returnValue);
            }

            [[nodiscard]] constexpr auto get_return_object() { return Task{CoroutineHandle::from_promise(*this)}; }

            template<typename... Args>
                requires std::constructible_from<T, Args...>
            constexpr auto return_value(Args &&...args) {
                this->returnValue.set_value(T{std::forward<Args>(args)...});
            }

            [[nodiscard]] constexpr auto getReturnValue() { return this->returnValue.get_future(); }

        private:
            std::promise<T> returnValue;
        };

    public:
        using promise_type = Promise;

        Task(const Task &) = delete;

        auto operator=(const Task &) -> Task & = delete;

        constexpr Task(Task &&) noexcept = default;

        constexpr auto operator=(Task &&) noexcept -> Task & = default;

        constexpr ~Task() = default;

        constexpr auto swap(Task &other) noexcept {
            std::swap(static_cast<BaseTask &>(*this), static_cast<BaseTask &>(other));
            std::swap(this->returnValue, other.returnValue);
        }

        [[nodiscard]] constexpr auto getReturnValue() noexcept -> std::future<T> & { return this->returnValue; }

        [[nodiscard]] constexpr auto await_resume() { return this->returnValue.get(); }

    private:
        explicit constexpr Task(const CoroutineHandle coroutineHandle) :
            BaseTask{internal::Coroutine{internal::Coroutine::Handle::from_address(coroutineHandle.address())}},
            returnValue{coroutineHandle.promise().getReturnValue()} {}

        std::future<T> returnValue;
    };

    template<typename T>
    class Task<T &> : public internal::BaseTask {
        class Promise;

        using CoroutineHandle = std::coroutine_handle<Promise>;

        class Promise : public internal::BasePromise {
        public:
            constexpr Promise() = default;

            Promise(const Promise &) = delete;

            auto operator=(const Promise &) -> Promise & = delete;

            constexpr Promise(Promise &&) noexcept = default;

            constexpr auto operator=(Promise &&) noexcept -> Promise & = default;

            constexpr ~Promise() = default;

            constexpr auto swap(Promise &other) noexcept {
                std::swap(static_cast<BasePromise &>(*this), static_cast<BasePromise &>(other));
                std::swap(this->returnValue, other.returnValue);
            }

            [[nodiscard]] constexpr auto get_return_object() { return Task{CoroutineHandle::from_promise(*this)}; }

            constexpr auto return_value(T &returnValue) { this->returnValue.set_value(returnValue); }

            [[nodiscard]] constexpr auto getReturnValue() { return this->returnValue.get_future(); }

        private:
            std::promise<T &> returnValue;
        };

    public:
        using promise_type = Promise;

        Task(const Task &) = delete;

        auto operator=(const Task &) -> Task & = delete;

        constexpr Task(Task &&) noexcept = default;

        constexpr auto operator=(Task &&) noexcept -> Task & = default;

        constexpr ~Task() = default;

        constexpr auto swap(Task &other) noexcept {
            std::swap(static_cast<BaseTask &>(*this), static_cast<BaseTask &>(other));
            std::swap(this->returnValue, other.returnValue);
        }

        [[nodiscard]] constexpr auto getReturnValue() noexcept -> std::future<T &> & { return this->returnValue; }

        [[nodiscard]] constexpr auto await_resume() -> T & { return this->returnValue.get(); }

    private:
        explicit constexpr Task(const CoroutineHandle coroutineHandle) :
            BaseTask{internal::Coroutine{internal::Coroutine::Handle::from_address(coroutineHandle.address())}},
            returnValue{coroutineHandle.promise().getReturnValue()} {}

        std::future<T &> returnValue;
    };

    template<>
    class Task<> : public internal::BaseTask {
        class Promise;

        using CoroutineHandle = std::coroutine_handle<Promise>;

        class Promise : public internal::BasePromise {
        public:
            Promise() = default;

            Promise(const Promise &) = delete;

            auto operator=(const Promise &) -> Promise & = delete;

            Promise(Promise &&) noexcept = default;

            auto operator=(Promise &&) noexcept -> Promise & = default;

            ~Promise() = default;

            constexpr auto swap(Promise &other) noexcept {
                std::swap(static_cast<BasePromise &>(*this), static_cast<BasePromise &>(other));
                std::swap(this->returnValue, other.returnValue);
            }

            [[nodiscard]] constexpr auto get_return_object() { return Task{CoroutineHandle::from_promise(*this)}; }

            constexpr auto return_void() { this->returnValue.set_value(); }

            [[nodiscard]] constexpr auto getReturnValue() { return this->returnValue.get_future(); }

        private:
            std::promise<void> returnValue;
        };

    public:
        using promise_type = Promise;

        Task(const Task &) = delete;

        auto operator=(const Task &) -> Task & = delete;

        Task(Task &&) noexcept = default;

        auto operator=(Task &&) noexcept -> Task & = default;

        ~Task() = default;

        constexpr auto swap(Task &other) noexcept {
            std::swap(static_cast<BaseTask &>(*this), static_cast<BaseTask &>(other));
            std::swap(this->returnValue, other.returnValue);
        }

        [[nodiscard]] constexpr auto getReturnValue() noexcept -> std::future<void> & { return this->returnValue; }

        constexpr auto await_resume() const noexcept {}

    private:
        explicit constexpr Task(const CoroutineHandle coroutineHandle) :
            BaseTask{internal::Coroutine{internal::Coroutine::Handle::from_address(coroutineHandle.address())}},
            returnValue{coroutineHandle.promise().getReturnValue()} {}

        std::future<void> returnValue;
    };
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

    template<typename T>
        requires std::is_lvalue_reference_v<T>
    constexpr auto swap(typename coContext::Task<T &>::Promise &lhs,
                        typename coContext::Task<T &>::Promise &rhs) noexcept -> void {
        lhs.swap(rhs);
    }

    template<typename T>
        requires std::is_lvalue_reference_v<T>
    constexpr auto swap(coContext::Task<T &> &lhs, coContext::Task<T &> &rhs) noexcept -> void {
        lhs.swap(rhs);
    }
}    // namespace std

template<>
constexpr auto std::swap(coContext::Task<> &lhs, coContext::Task<> &rhs) noexcept -> void {
    lhs.swap(rhs);
}

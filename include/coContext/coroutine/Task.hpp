#pragma once

#include "BasePromise.hpp"
#include "BaseTask.hpp"

namespace coContext {
    template<typename T = void>
        requires std::is_object_v<T> || std::is_void_v<T>
    class Task : public BaseTask {
    public:
        class Promise;

        using promise_type = Promise;
        using CoroutineHandle = std::coroutine_handle<Promise>;

        class Promise : public BasePromise {
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
            constexpr auto return_value(Args &&...args) const {
                std::construct_at(this->returnValue.get(), std::forward<Args>(args)...);
            }

            [[nodiscard]] constexpr auto getReturnValue() const noexcept { return this->returnValue; }

        private:
            std::shared_ptr<T> returnValue{
                std::allocate_shared_for_overwrite<T>(std::pmr::polymorphic_allocator<T>{getMemoryResource()})};
        };

        explicit constexpr Task(const CoroutineHandle coroutineHandle) :
            BaseTask{Coroutine{Coroutine::Handle::from_address(coroutineHandle.address())}},
            returnValue{coroutineHandle.promise().getReturnValue()} {}

        Task(const Task &) = delete;

        auto operator=(const Task &) -> Task & = delete;

        constexpr Task(Task &&) noexcept = default;

        constexpr auto operator=(Task &&) noexcept -> Task & = default;

        constexpr ~Task() = default;

        constexpr auto swap(Task &other) noexcept {
            std::swap(static_cast<BaseTask &>(*this), static_cast<BaseTask &>(other));
            std::swap(this->returnValue, other.returnValue);
        }

        [[nodiscard]] constexpr auto getReturnValue() const noexcept { return this->returnValue; }

        [[nodiscard]] constexpr auto await_resume() const noexcept -> T & { return *this->returnValue; }

    private:
        std::shared_ptr<T> returnValue;
    };

    template<>
    class Task<> : public BaseTask {
    public:
        class Promise;

        using promise_type = Promise;
        using CoroutineHandle = std::coroutine_handle<Promise>;

        class Promise : public BasePromise {
        public:
            Promise() = default;

            Promise(const Promise &) = delete;

            auto operator=(const Promise &) -> Promise & = delete;

            Promise(Promise &&) noexcept = default;

            auto operator=(Promise &&) noexcept -> Promise & = default;

            ~Promise() = default;

            constexpr auto swap(Promise &other) noexcept {
                std::swap(static_cast<BasePromise &>(*this), static_cast<BasePromise &>(other));
            }

            [[nodiscard]] constexpr auto get_return_object() { return Task{CoroutineHandle::from_promise(*this)}; }

            constexpr auto return_void() const noexcept {}
        };

        explicit constexpr Task(const CoroutineHandle coroutineHandle) noexcept :
            BaseTask{Coroutine{Coroutine::Handle::from_address(coroutineHandle.address())}} {}

        Task(const Task &) = delete;

        auto operator=(const Task &) -> Task & = delete;

        Task(Task &&) noexcept = default;

        auto operator=(Task &&) noexcept -> Task & = default;

        ~Task() = default;

        constexpr auto swap(Task &other) noexcept {
            std::swap(static_cast<BaseTask &>(*this), static_cast<BaseTask &>(other));
        }

        constexpr auto await_resume() const noexcept {}
    };
}    // namespace coContext

namespace std {
    template<typename T>
        requires std::is_object_v<T>
    constexpr auto swap(typename coContext::Task<T>::Promise &lhs, typename coContext::Task<T>::Promise &rhs) noexcept
        -> void {
        lhs.swap(rhs);
    }

    template<typename T>
        requires std::is_object_v<T>
    constexpr auto swap(coContext::Task<T> &lhs, coContext::Task<T> &rhs) noexcept -> void {
        lhs.swap(rhs);
    }
}    // namespace std

template<>
constexpr auto std::swap(coContext::Task<>::Promise &lhs, coContext::Task<>::Promise &rhs) noexcept -> void {
    lhs.swap(rhs);
}

template<>
constexpr auto std::swap(coContext::Task<> &lhs, coContext::Task<> &rhs) noexcept -> void {
    lhs.swap(rhs);
}

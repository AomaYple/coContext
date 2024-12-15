#pragma once

#include "Coroutine.hpp"

#include <future>
#include <memory_resource>

namespace coContext {
    class BasePromise {
    public:
        [[nodiscard]] auto operator new(std::size_t numberOfBytes) -> void *;

        auto operator delete(void *pointer, std::size_t numberOfBytes) noexcept -> void;

        BasePromise() = default;

        BasePromise(const BasePromise &) = delete;

        auto operator=(const BasePromise &) -> BasePromise & = delete;

        BasePromise(BasePromise &&) noexcept = default;

        auto operator=(BasePromise &&) noexcept -> BasePromise & = default;

        ~BasePromise() = default;

        auto swap(BasePromise &other) noexcept -> void;

        [[nodiscard]] auto getResult() -> std::future<std::int32_t>;

        auto setResult(std::int32_t result) -> void;

        [[nodiscard]] auto getParentCoroutineIdentity() const noexcept -> std::uint64_t;

        auto setParentCoroutineIdentity(std::uint64_t identity) noexcept -> void;

        [[nodiscard]] auto getChildCoroutine() noexcept -> Coroutine &;

        auto setChildCoroutine(Coroutine &&coroutine) noexcept -> void;

        [[nodiscard]] auto initial_suspend() const noexcept -> std::suspend_always;

        [[nodiscard]] auto final_suspend() const noexcept -> std::suspend_always;

        auto unhandled_exception() const -> void;

    private:
        static thread_local std::pmr::polymorphic_allocator<> allocator;

        std::promise<std::int32_t> result;
        std::uint64_t parentCoroutineIdentity{std::hash<Coroutine>{}(Coroutine{})};
        Coroutine childCoroutine;
    };
}    // namespace coContext

template<>
constexpr auto std::swap(coContext::BasePromise &lhs, coContext::BasePromise &rhs) noexcept -> void {
    lhs.swap(rhs);
}

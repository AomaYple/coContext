#pragma once

#include "Coroutine.hpp"

#include <memory_resource>

namespace coContext {
    class BasePromise {
    public:
        [[nodiscard]] auto operator new(std::size_t) -> void *;

        auto operator delete(void *, std::size_t) noexcept -> void;

        BasePromise(const BasePromise &) = delete;

        auto operator=(const BasePromise &) -> BasePromise & = delete;

        BasePromise(BasePromise &&) noexcept = default;

        auto operator=(BasePromise &&) noexcept -> BasePromise & = default;

        ~BasePromise() = default;

        auto swap(BasePromise &other) noexcept -> void;

        [[nodiscard]] auto getResult() const noexcept -> std::int32_t;

        auto setResult(std::int32_t result) noexcept -> void;

        [[nodiscard]] auto getParentCoroutineIdentity() const noexcept -> std::uint64_t;

        auto setParentCoroutineIdentity(std::uint64_t identity) noexcept -> void;

        [[nodiscard]] auto getChildCoroutine() noexcept -> Coroutine &;

        auto setChildCoroutine(Coroutine &&coroutine) noexcept -> void;

        [[nodiscard]] auto initial_suspend() const noexcept -> std::suspend_always;

        [[nodiscard]] auto final_suspend() const noexcept -> std::suspend_always;

        auto unhandled_exception() const -> void;

    protected:
        constexpr BasePromise() noexcept = default;

    private:
        static thread_local std::pmr::polymorphic_allocator<> allocator;

        std::int32_t result{};
        std::uint64_t parentCoroutineIdentity{std::hash<Coroutine>{}(Coroutine{nullptr})};
        Coroutine childCoroutine{nullptr};
    };
}    // namespace coContext

template<>
constexpr auto std::swap(coContext::BasePromise &lhs, coContext::BasePromise &rhs) noexcept -> void {
    lhs.swap(rhs);
}

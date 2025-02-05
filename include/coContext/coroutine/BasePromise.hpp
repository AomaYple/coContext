#pragma once

#include "../memory/memoryResource.hpp"
#include "Coroutine.hpp"

#include <memory>

namespace coContext::internal {
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

        [[nodiscard]] auto getFlags() const noexcept -> std::uint32_t;

        auto setFlags(std::uint32_t flags) noexcept -> void;

        [[nodiscard]] auto getException() const noexcept -> const std::shared_ptr<std::exception_ptr> &;

        [[nodiscard]] auto getParentCoroutineId() const noexcept -> std::uint64_t;

        auto setParentCoroutineId(std::uint64_t id) noexcept -> void;

        [[nodiscard]] auto getChildCoroutine() noexcept -> Coroutine &;

        auto setChildCoroutine(Coroutine coroutine) noexcept -> void;

        [[nodiscard]] auto initial_suspend() const noexcept -> std::suspend_always;

        [[nodiscard]] auto final_suspend() const noexcept -> std::suspend_always;

        auto unhandled_exception() const noexcept -> void;

    protected:
        constexpr BasePromise() noexcept = default;

    private:
        std::int32_t result{};
        std::uint32_t flags{};
        std::shared_ptr<std::exception_ptr> exception{std::allocate_shared<std::exception_ptr>(
            std::pmr::polymorphic_allocator<std::exception_ptr>{getUnsyncMemoryResource()})};
        std::uint64_t parentCoroutineId{std::hash<Coroutine>{}(Coroutine{nullptr})};
        Coroutine childCoroutine{nullptr};
    };
}    // namespace coContext::internal

template<>
constexpr auto std::swap(coContext::internal::BasePromise &lhs, coContext::internal::BasePromise &rhs) noexcept
    -> void {
    lhs.swap(rhs);
}

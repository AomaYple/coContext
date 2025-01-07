#pragma once

#include <coroutine>

namespace coContext::internal {
    class BasePromise;

    class Coroutine {
    public:
        using Handle = std::coroutine_handle<BasePromise>;

        explicit Coroutine(Handle handle) noexcept;

        Coroutine(const Coroutine &) = delete;

        auto operator=(const Coroutine &) -> Coroutine & = delete;

        Coroutine(Coroutine &&) noexcept;

        auto operator=(Coroutine &&) noexcept -> Coroutine &;

        ~Coroutine();

        auto swap(Coroutine &other) noexcept -> void;

        [[nodiscard]] auto get() const noexcept -> Handle;

        explicit operator bool() const noexcept;

        [[nodiscard]] auto getPromise() const -> BasePromise &;

        auto operator()() const -> void;

        [[nodiscard]] auto isDone() const noexcept -> bool;

    private:
        Handle handle;
    };
}    // namespace coContext::internal

template<>
constexpr auto std::swap(coContext::internal::Coroutine &lhs, coContext::internal::Coroutine &rhs) noexcept -> void {
    lhs.swap(rhs);
}

template<>
struct std::hash<coContext::internal::Coroutine> {
    [[nodiscard]] constexpr auto operator()(const coContext::internal::Coroutine &coroutine) const noexcept {
        return std::hash<coContext::internal::Coroutine::Handle>{}(coroutine.get());
    }
};

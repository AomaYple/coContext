#pragma once

#include <coroutine>

namespace coContext {
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

        [[nodiscard]] auto promise() const -> BasePromise &;

        auto operator()() const -> void;

        [[nodiscard]] auto done() const noexcept -> bool;

    private:
        auto destroy() const -> void;

        Handle handle;
    };
}    // namespace coContext

template<>
constexpr auto std::swap(coContext::Coroutine &lhs, coContext::Coroutine &rhs) noexcept -> void {
    lhs.swap(rhs);
}

template<>
struct std::hash<coContext::Coroutine> {
    [[nodiscard]] constexpr auto operator()(const coContext::Coroutine &coroutine) const noexcept {
        return std::hash<coContext::Coroutine::Handle>{}(coroutine.get());
    }
};

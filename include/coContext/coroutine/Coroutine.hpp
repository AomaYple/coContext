#pragma once

#include <coroutine>

namespace coContext {
    class Coroutine {
    public:
        explicit Coroutine(std::coroutine_handle<> handle) noexcept;

        Coroutine(const Coroutine &) = delete;

        auto operator=(const Coroutine &) -> Coroutine & = delete;

        Coroutine(Coroutine &&) noexcept;

        auto operator=(Coroutine &&) noexcept -> Coroutine &;

        ~Coroutine();

        auto swap(Coroutine &other) noexcept -> void;

        [[nodiscard]] auto get() const noexcept -> std::coroutine_handle<>;

        auto operator()() const -> void;

        [[nodiscard]] auto done() const noexcept -> bool;

    private:
        auto destroy() const -> void;

        std::coroutine_handle<> handle;
    };
}    // namespace coContext

template<>
constexpr auto std::swap(coContext::Coroutine &lhs, coContext::Coroutine &rhs) noexcept -> void {
    lhs.swap(rhs);
}

template<>
struct std::hash<coContext::Coroutine> {
    [[nodiscard]] auto operator()(const coContext::Coroutine &coroutine) const noexcept {
        return std::hash<std::coroutine_handle<>>{}(coroutine.get());
    }
};

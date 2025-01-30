#pragma once

#include <cstdint>
#include <functional>

namespace coContext::internal {
    class AsyncWaiter;

    class Marker {
        using Action = std::function<auto()->void>;

    public:
        explicit Marker(std::uint32_t flags = {}, Action action = {}) noexcept;

        Marker(const Marker &) = default;

        auto operator=(const Marker &) -> Marker & = default;

        Marker(Marker &&) noexcept = default;

        auto operator=(Marker &&) noexcept -> Marker & = default;

        ~Marker() = default;

        auto swap(Marker &other) noexcept -> void;

        [[nodiscard]] auto getFlags() const noexcept -> std::uint32_t;

        [[nodiscard]] auto getAction() noexcept -> Action &;

        auto addFlags(std::uint32_t flags) noexcept -> void;

        auto setAction(Action action) noexcept -> void;

        auto executeAction() const -> void;

    private:
        std::uint32_t flags;
        Action action;
    };

    [[nodiscard]] auto operator|(Marker, Marker) noexcept -> Marker;

    [[nodiscard]] auto operator|(AsyncWaiter, const Marker &) -> AsyncWaiter;
}    // namespace coContext::internal

template<>
constexpr auto std::swap(coContext::internal::Marker &lhs, coContext::internal::Marker &rhs) noexcept -> void {
    lhs.swap(rhs);
}

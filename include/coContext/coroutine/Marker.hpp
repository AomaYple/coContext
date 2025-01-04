#pragma once

#include <cstdint>
#include <functional>
#include <span>

namespace coContext::internal {
    class AsyncWaiter;

    class Marker {
        using Action = std::move_only_function<auto()->void>;

    public:
        explicit Marker(std::uint32_t flags = {}, Action action = {});

        Marker(const Marker &) = delete;

        auto operator=(const Marker &) -> Marker & = delete;

        constexpr Marker(Marker &&) noexcept = default;

        constexpr auto operator=(Marker &&) noexcept -> Marker & = default;

        constexpr ~Marker() = default;

        auto swap(Marker &other) noexcept -> void;

        [[nodiscard]] auto getFlags() const noexcept -> std::uint32_t;

        [[nodiscard]] auto getActions() noexcept -> std::span<Action>;

        auto addFlags(std::uint32_t flags) noexcept -> void;

        auto addActions(std::span<Action> actions) -> void;

        auto executeActions() -> void;

    private:
        std::uint32_t flags;
        std::vector<Action> actions;
    };

    [[nodiscard]] auto operator|(Marker, Marker) -> Marker;

    [[nodiscard]] auto operator|(AsyncWaiter, Marker) -> AsyncWaiter;
}    // namespace coContext::internal

template<>
constexpr auto std::swap(coContext::internal::Marker &lhs, coContext::internal::Marker &rhs) noexcept -> void {
    lhs.swap(rhs);
}

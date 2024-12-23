#pragma once

#include <cstdint>

namespace coContext::internal {
    class AsyncWaiter;

    class Marker {
    public:
        explicit Marker(std::uint32_t flags = {}) noexcept;

        constexpr Marker(const Marker &) noexcept = default;

        constexpr auto operator=(const Marker &) noexcept -> Marker & = default;

        constexpr Marker(Marker &&) noexcept = default;

        constexpr auto operator=(Marker &&) noexcept -> Marker & = default;

        constexpr ~Marker() = default;

        [[nodiscard]] auto getFlags() const noexcept -> std::uint32_t;

    private:
        std::uint32_t flags;
    };

    [[nodiscard]] auto operator==(const Marker &, const Marker &) noexcept -> bool;

    [[nodiscard]] auto operator|(AsyncWaiter, Marker) noexcept -> AsyncWaiter;
}    // namespace coContext::internal

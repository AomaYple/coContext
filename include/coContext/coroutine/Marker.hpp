#pragma once

#include <linux/time_types.h>
#include <memory>

namespace coContext {
    class AsyncWaiter;

    class Marker {
    public:
        explicit Marker(std::uint32_t flags) noexcept;

        Marker(const Marker &) = delete;

        auto operator=(const Marker &) -> Marker & = delete;

        constexpr Marker(Marker &&) noexcept = default;

        constexpr auto operator=(Marker &&) noexcept -> Marker & = default;

        constexpr ~Marker() = default;

        auto swap(Marker &other) noexcept -> void;

        [[nodiscard]] auto getFlags() const noexcept -> std::uint32_t;

        [[nodiscard]] auto getTimeSpecification() noexcept -> std::unique_ptr<__kernel_timespec> &;

        auto setTimeSpecification(std::unique_ptr<__kernel_timespec> &&timeSpecification) noexcept -> void;

    private:
        std::uint32_t flags;
        std::unique_ptr<__kernel_timespec> timeSpecification;
    };

    [[nodiscard]] auto operator|(AsyncWaiter &&asyncWaiter, Marker &&marker) noexcept -> AsyncWaiter;
}    // namespace coContext

template<>
constexpr auto std::swap(coContext::Marker &lhs, coContext::Marker &rhs) noexcept -> void {
    lhs.swap(rhs);
}

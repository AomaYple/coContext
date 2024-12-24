#include "coContext/coroutine/Marker.hpp"

#include "coContext/coroutine/AsyncWaiter.hpp"

coContext::internal::Marker::Marker(const std::uint32_t flags) noexcept : flags{flags} {}

auto coContext::internal::Marker::getFlags() const noexcept -> std::uint32_t { return this->flags; }

auto coContext::internal::operator==(const Marker &lhs, const Marker &rhs) noexcept -> bool {
    return lhs.getFlags() == rhs.getFlags();
}

auto coContext::internal::operator|(AsyncWaiter asyncWaiter, const Marker marker) noexcept -> AsyncWaiter {
    asyncWaiter.getSubmission().addFlags(marker.getFlags());

    return asyncWaiter;
}

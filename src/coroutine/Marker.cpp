#include "coContext/coroutine/Marker.hpp"

#include "coContext/coroutine/AsyncWaiter.hpp"

coContext::Marker::Marker(const std::uint32_t flags) noexcept : flags{flags} {}

auto coContext::Marker::getFlags() const noexcept -> std::uint32_t { return this->flags; }

auto coContext::operator==(const Marker &lhs, const Marker &rhs) noexcept -> bool {
    return lhs.getFlags() == rhs.getFlags();
}

auto coContext::operator|(AsyncWaiter asyncWaiter, const Marker marker) noexcept -> AsyncWaiter {
    asyncWaiter.getSubmissionQueueEntry().addFlags(marker.getFlags());

    return asyncWaiter;
}

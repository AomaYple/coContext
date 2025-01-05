#include "coContext/coroutine/Marker.hpp"

#include "coContext/coroutine/AsyncWaiter.hpp"

coContext::internal::Marker::Marker(const std::uint32_t flags, Action action) :
    flags{flags}, action{std::move(action)} {}

auto coContext::internal::Marker::swap(Marker &other) noexcept -> void {
    std::swap(this->flags, other.flags);
    std::swap(this->action, other.action);
}

auto coContext::internal::Marker::getFlags() const noexcept -> std::uint32_t { return this->flags; }

auto coContext::internal::Marker::getAction() noexcept -> Action & { return this->action; }

auto coContext::internal::Marker::addFlags(const std::uint32_t flags) noexcept -> void { this->flags |= flags; }

auto coContext::internal::Marker::setAction(Action action) -> void { this->action = std::move(action); }

auto coContext::internal::Marker::executeAction() -> void {
    if (static_cast<bool>(this->action)) this->action();
}

auto coContext::internal::operator|(Marker lhs, Marker rhs) -> Marker {
    rhs.addFlags(lhs.getFlags());
    rhs.setAction(std::move(lhs.getAction()));

    return rhs;
}

auto coContext::internal::operator|(AsyncWaiter asyncWaiter, Marker marker) -> AsyncWaiter {
    asyncWaiter.getSubmission().addFlags(marker.getFlags());
    marker.executeAction();

    return asyncWaiter;
}

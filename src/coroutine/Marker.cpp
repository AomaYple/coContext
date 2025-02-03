#include "coContext/coroutine/Marker.hpp"

#include "coContext/coroutine/AsyncWaiter.hpp"

coContext::internal::Marker::Marker(const std::uint32_t flags, Action action) noexcept :
    flags{flags}, action{std::move(action)} {}

auto coContext::internal::Marker::swap(Marker &other) noexcept -> void {
    std::swap(this->flags, other.flags);
    std::swap(this->action, other.action);
}

auto coContext::internal::Marker::getFlags() const noexcept -> std::uint32_t { return this->flags; }

auto coContext::internal::Marker::getAction() noexcept -> Action & { return this->action; }

auto coContext::internal::Marker::addFlags(const std::uint32_t flags) noexcept -> void { this->flags |= flags; }

auto coContext::internal::Marker::setAction(Action action) noexcept -> void { this->action = std::move(action); }

auto coContext::internal::Marker::executeAction() const -> void {
    if (this->action) this->action();
}

auto coContext::internal::operator|(Marker lhs, Marker rhs) noexcept -> Marker {
    lhs.addFlags(rhs.getFlags());
    lhs.setAction(std::move(rhs.getAction()));

    return rhs;
}

auto coContext::internal::operator|(AsyncWaiter asyncWaiter, const Marker &marker) -> AsyncWaiter {
    asyncWaiter.getSubmission().addFlags(marker.getFlags());
    marker.executeAction();

    return asyncWaiter;
}

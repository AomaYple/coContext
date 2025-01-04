#include "coContext/coroutine/Marker.hpp"

#include "coContext/coroutine/AsyncWaiter.hpp"

coContext::internal::Marker::Marker(const std::uint32_t flags, Action action) : flags{flags} {
    if (static_cast<bool>(action)) this->actions.emplace_back(std::move(action));
}

auto coContext::internal::Marker::swap(Marker &other) noexcept -> void {
    std::swap(this->flags, other.flags);
    std::swap(this->actions, other.actions);
}

auto coContext::internal::Marker::getFlags() const noexcept -> std::uint32_t { return this->flags; }

auto coContext::internal::Marker::getActions() noexcept -> std::span<Action> { return this->actions; }

auto coContext::internal::Marker::addFlags(const std::uint32_t flags) noexcept -> void { this->flags |= flags; }

auto coContext::internal::Marker::addActions(const std::span<Action> actions) -> void {
    std::ranges::move(actions, std::back_inserter(this->actions));
}

auto coContext::internal::Marker::executeActions() -> void {
    for (auto &action : this->actions) action();
}

auto coContext::internal::operator|(Marker lhs, Marker rhs) -> Marker {
    rhs.addFlags(lhs.getFlags());
    rhs.addActions(lhs.getActions());

    return rhs;
}

auto coContext::internal::operator|(AsyncWaiter asyncWaiter, Marker marker) -> AsyncWaiter {
    asyncWaiter.getSubmission().addFlags(marker.getFlags());
    marker.executeActions();

    return asyncWaiter;
}

#include "coContext/coroutine/Marker.hpp"

#include "coContext/coroutine/AsyncWaiter.hpp"

coContext::Marker::Marker(const std::uint32_t flags) noexcept : flags{flags} {}

auto coContext::Marker::swap(Marker &other) noexcept -> void {
    std::swap(this->flags, other.flags);
    std::swap(this->timeSpecification, other.timeSpecification);
}

auto coContext::Marker::getFlags() const noexcept -> std::uint32_t { return this->flags; }

auto coContext::Marker::getTimeSpecification() noexcept -> std::unique_ptr<__kernel_timespec> & {
    return this->timeSpecification;
}

auto coContext::Marker::setTimeSpecification(std::unique_ptr<__kernel_timespec> &&timeSpecification) noexcept -> void {
    this->timeSpecification = std::move(timeSpecification);
}

auto coContext::operator|(AsyncWaiter &&asyncWaiter, Marker &&marker) noexcept -> AsyncWaiter {
    asyncWaiter.getSubmissionQueueEntry().addFlags(marker.getFlags());
    asyncWaiter.getTimeSpecifications().second = std::move(marker.getTimeSpecification());

    return asyncWaiter;
}

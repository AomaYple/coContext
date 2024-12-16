#include "Exception.hpp"

#include "../memory/memoryResource.hpp"

coContext::Exception::Exception(Log &&log, const std::pmr::polymorphic_allocator<char> allocator) :
    message{log.toString(allocator), allocator}, log{std::move(log)} {}

auto coContext::Exception::swap(Exception &other) noexcept -> void {
    std::swap(this->message, other.message);
    std::swap(this->log, other.log);
}

auto coContext::Exception::what() const noexcept -> const char * { return std::data(this->message); }

auto coContext::Exception::getLog() noexcept -> Log & { return this->log; }

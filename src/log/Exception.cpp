#include "Exception.hpp"

coContext::Exception::Exception(Log &&log) : message{log.toString(), getMemoryResource()}, log{std::move(log)} {}

auto coContext::Exception::swap(Exception &other) noexcept -> void {
    std::swap(this->message, other.message);
    std::swap(this->log, other.log);
}

auto coContext::Exception::what() const noexcept -> const char * { return std::data(this->message); }

auto coContext::Exception::getLog() noexcept -> Log & { return this->log; }

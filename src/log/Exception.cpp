#include "Exception.hpp"

coContext::Exception::Exception(Log &&log) : message{log.toString()}, log{std::move(log)} {}

auto coContext::Exception::what() const noexcept -> const char * { return this->message.c_str(); }

auto coContext::Exception::getLog() noexcept -> Log & { return this->log; }

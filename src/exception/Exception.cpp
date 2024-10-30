#include "Exception.hpp"

coRing::Exception::Exception(std::string &&message) noexcept : message{std::move(message)} {}

auto coRing::Exception::what() const noexcept -> const char * { return this->message.c_str(); };

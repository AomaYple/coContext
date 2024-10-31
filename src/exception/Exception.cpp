#include "Exception.hpp"

coContext::Exception::Exception(const char *const message) : message{message} {}

coContext::Exception::Exception(const std::string_view message) : message{message} {}

coContext::Exception::Exception(std::string &&message) noexcept : message{std::move(message)} {}

auto coContext::Exception::what() const noexcept -> const char * { return this->message.c_str(); };

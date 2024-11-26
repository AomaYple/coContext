#include "Exception.hpp"

coContext::Exception::Exception(std::string &&message) noexcept : message{std::move(message)} {}

auto coContext::Exception::swap(Exception &other) noexcept -> void { std::swap(this->message, other.message); }

auto coContext::Exception::what() const noexcept -> const char * { return this->message.c_str(); }

auto coContext::Exception::getMessage() noexcept -> std::string & { return this->message; }

auto coContext::operator==(const Exception &lhs, const Exception &rhs) noexcept -> bool {
    return lhs.getMessage() == rhs.getMessage();
}


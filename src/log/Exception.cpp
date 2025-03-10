#include "Exception.hpp"

using namespace std::string_view_literals;

coContext::Exception::Exception(Log log) : message{std::format("{}"sv, log)}, log{std::move(log)} {}

auto coContext::Exception::swap(Exception &other) noexcept -> void {
    std::swap(this->message, other.message);
    std::swap(this->log, other.log);
}

auto coContext::Exception::what() const noexcept -> const char * { return std::data(this->message); }

auto coContext::Exception::getMessage() const noexcept -> std::string_view { return this->message; }

auto coContext::Exception::getLog() const noexcept -> const Log & { return this->log; }

auto coContext::Exception::getLog() noexcept -> Log & { return this->log; }

auto coContext::operator==(const Exception &lhs, const Exception &rhs) noexcept -> bool {
    return lhs.getMessage() == rhs.getMessage() && lhs.getLog() == rhs.getLog();
}

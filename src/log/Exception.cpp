#include "Exception.hpp"

using namespace std::string_view_literals;

coContext::internal::Exception::Exception(Log log) :
    message{std::format("{}"sv, log), getSyncMemoryResource()}, log{std::move(log)} {}

auto coContext::internal::Exception::swap(Exception &other) noexcept -> void {
    std::swap(this->message, other.message);
    std::swap(this->log, other.log);
}

auto coContext::internal::Exception::what() const noexcept -> const char * { return std::data(this->message); }

auto coContext::internal::Exception::getMessage() const noexcept -> std::string_view { return this->message; }

auto coContext::internal::Exception::getLog() const noexcept -> const Log & { return this->log; }

auto coContext::internal::Exception::getLog() noexcept -> Log & { return this->log; }

auto coContext::internal::operator==(const Exception &lhs, const Exception &rhs) noexcept -> bool {
    return lhs.getMessage() == rhs.getMessage() && lhs.getLog() == rhs.getLog();
}

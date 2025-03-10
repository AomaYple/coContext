#include "coContext/log/Log.hpp"

coContext::Log::Log(const Level level, std::pmr::string message, const std::source_location sourceLocation,
                    const std::chrono::system_clock::time_point timestamp, const std::thread::id threadId) :
    level{level}, timestamp{timestamp}, threadId{threadId}, sourceLocation{sourceLocation},
    message{std::move(message), getSyncMemoryResource()} {}

auto coContext::Log::swap(Log &other) noexcept -> void {
    std::swap(this->level, other.level);
    std::swap(this->timestamp, other.timestamp);
    std::swap(this->threadId, other.threadId);
    std::swap(this->sourceLocation, other.sourceLocation);
    std::swap(this->message, other.message);
}

auto coContext::Log::getLevel() const noexcept -> Level { return this->level; }

auto coContext::Log::getTimeStamp() const noexcept -> std::chrono::system_clock::time_point { return this->timestamp; }

auto coContext::Log::getThreadId() const noexcept -> std::thread::id { return this->threadId; }

auto coContext::Log::getSourceLocation() const noexcept -> std::source_location { return this->sourceLocation; }

auto coContext::Log::getMessage() const noexcept -> std::string_view { return this->message; }

auto coContext::operator==(const Log &lhs, const Log &rhs) noexcept -> bool {
    const std::source_location lhsSourceLocation{lhs.getSourceLocation()}, rhsSourceLocation{rhs.getSourceLocation()};

    return lhs.getLevel() == rhs.getLevel() && lhs.getTimeStamp() == rhs.getTimeStamp() &&
           lhs.getThreadId() == rhs.getThreadId() && lhsSourceLocation.file_name() == rhsSourceLocation.file_name() &&
           lhsSourceLocation.line() == rhsSourceLocation.line() &&
           lhsSourceLocation.column() == rhsSourceLocation.column() &&
           lhsSourceLocation.function_name() == rhsSourceLocation.function_name() &&
           lhs.getMessage() == rhs.getMessage();
}


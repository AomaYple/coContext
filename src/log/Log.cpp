#include "coContext/log/Log.hpp"

#include <utility>

using namespace std::string_view_literals;

auto coContext::Log::formatLevel(const Level level) noexcept -> std::string_view {
    static constexpr std::array<const std::string_view, 6> levels{"trace"sv, "debug"sv, "info"sv,
                                                                  "warn"sv,  "error"sv, "fatal"sv};

    return levels[std::to_underlying(level)];
}

coContext::Log::Log(const Level level, std::pmr::string message, const std::source_location sourceLocation,
                    const std::chrono::system_clock::time_point timestamp, const std::thread::id threadId) :
    level{level}, timestamp{timestamp}, threadId{threadId}, sourceLocation{sourceLocation},
    message{std::move(message), internal::getSyncMemoryResource()} {}

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

auto coContext::operator<<(std::ostream &outputStream, const Log &log) -> std::ostream & {
    return outputStream << std::format("{}"sv, log);
}

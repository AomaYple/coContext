#include "Log.hpp"

#include <chrono>
#include <utility>

coContext::Log::Log(const Level level, std::string &&message, const std::source_location sourceLocation,
                    const std::chrono::system_clock::time_point timestamp, const std::thread::id threadId) noexcept :
    level{level}, message{std::move(message)}, sourceLocation{sourceLocation}, timestamp{timestamp},
    threadId{threadId} {}

auto coContext::Log::swap(Log &other) noexcept -> void {
    std::swap(this->level, other.level);
    std::swap(this->message, other.message);
    std::swap(this->sourceLocation, other.sourceLocation);
    std::swap(this->timestamp, other.timestamp);
    std::swap(this->threadId, other.threadId);
}

auto coContext::Log::getLevel() const noexcept -> Level { return this->level; }

auto coContext::Log::toString() const -> std::string {
    static constexpr std::array<const std::string_view, 6> levels{"trace", "debug", "info", "warn", "error", "fatal"};

    return std::format("{} {} {} {}:{}:{}:{} {}\n", levels[std::to_underlying(this->level)], this->timestamp,
                       this->threadId, this->sourceLocation.file_name(), this->sourceLocation.line(),
                       this->sourceLocation.column(), this->sourceLocation.function_name(), this->message);
}

auto coContext::Log::toByte() const -> std::vector<std::byte> {
    const auto log{this->toString()};
    const auto bytes{std::as_bytes(std::span{log})};

    return {bytes.cbegin(), bytes.cend()};
}

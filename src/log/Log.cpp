#include "Log.hpp"

#include <chrono>
#include <utility>

using namespace std::string_view_literals;

coContext::Log::Log(const Level level, std::pmr::string message, const std::source_location sourceLocation,
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

auto coContext::Log::toString() const -> std::pmr::string {
    static constexpr std::array<const std::string_view, 6> levels{"trace"sv, "debug"sv, "info"sv,
                                                                  "warn"sv,  "error"sv, "fatal"sv};

    return std::pmr::string{std::format("{} {} {} {}:{}:{}:{} {}\n"sv, levels[std::to_underlying(this->level)],
                                        this->timestamp, this->threadId, this->sourceLocation.file_name(),
                                        this->sourceLocation.line(), this->sourceLocation.column(),
                                        this->sourceLocation.function_name(), this->message),
                            internal::getMemoryResource()};
}

auto coContext::Log::toBytes() const -> std::pmr::vector<std::byte> {
    const auto log{this->toString()};
    const auto bytes{std::as_bytes(std::span{log})};

    return {std::cbegin(bytes), std::cend(bytes), internal::getMemoryResource()};
}

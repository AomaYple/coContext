#pragma once

#include "../memory/memoryResource.hpp"

#include <source_location>
#include <thread>

namespace coContext {
    class Log {
    public:
        enum class Level : std::uint8_t { trace, debug, info, warn, error, fatal };

        explicit Log(Level level = Level::info,
                     std::pmr::string message = std::pmr::string{internal::getSyncMemoryResource()},
                     std::source_location sourceLocation = std::source_location::current(),
                     std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now(),
                     std::thread::id threadId = std::this_thread::get_id()) noexcept;

        auto swap(Log &other) noexcept -> void;

        [[nodiscard]] auto getLevel() const noexcept -> Level;

        [[nodiscard]] auto getFormattedLevel() const noexcept -> std::string_view;

        [[nodiscard]] auto getTimeStamp() const noexcept -> std::chrono::system_clock::time_point;

        [[nodiscard]] auto getThreadId() const noexcept -> std::thread::id;

        [[nodiscard]] auto getSourceLocation() const noexcept -> std::source_location;

        [[nodiscard]] auto getMessage() const noexcept -> std::string_view;

        [[nodiscard]] auto toString() const -> std::pmr::string;

    private:
        Level level;
        std::chrono::system_clock::time_point timestamp;
        std::thread::id threadId;
        std::source_location sourceLocation;
        std::pmr::string message;
    };
}    // namespace coContext

template<>
constexpr auto std::swap(coContext::Log &lhs, coContext::Log &rhs) noexcept -> void {
    lhs.swap(rhs);
}

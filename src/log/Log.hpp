#pragma once

#include "../memory/memoryResource.hpp"

#include <source_location>
#include <thread>

namespace coContext::internal {
    class Log {
    public:
        enum class Level : std::uint8_t { trace, debug, info, warn, error, fatal };

        explicit Log(Level level = Level::info, std::pmr::string message = std::pmr::string{getMemoryResource()},
                     std::source_location sourceLocation = std::source_location::current(),
                     std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now(),
                     std::thread::id threadId = std::this_thread::get_id()) noexcept;

        auto swap(Log &other) noexcept -> void;

        [[nodiscard]] auto getLevel() const noexcept -> Level;

        [[nodiscard]] auto toString() const -> std::pmr::string;

        [[nodiscard]] auto toBytes() const -> std::pmr::vector<std::byte>;

    private:
        Level level;
        std::pmr::string message;
        std::source_location sourceLocation;
        std::chrono::system_clock::time_point timestamp;
        std::thread::id threadId;
    };
}    // namespace coContext::internal

template<>
constexpr auto std::swap(coContext::internal::Log &lhs, coContext::internal::Log &rhs) noexcept -> void {
    lhs.swap(rhs);
}

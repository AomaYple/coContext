#pragma once

#include <source_location>
#include <thread>
#include <vector>

namespace coContext {
    class Log {
    public:
        enum class Level : std::uint8_t { info, warn, error, fatal };

        explicit Log(Level level = {}, std::string &&message = {},
                     std::source_location sourceLocation = std::source_location::current(),
                     std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now(),
                     std::jthread::id joinThreadId = std::this_thread::get_id()) noexcept;

        auto swap(Log &other) noexcept -> void;

        [[nodiscard]] auto toString() const -> std::string;

        [[nodiscard]] auto toByte() const -> std::vector<std::byte>;

    private:
        Level level;
        std::string message;
        std::source_location sourceLocation;
        std::chrono::system_clock::time_point timestamp;
        std::jthread::id joinThreadId;
    };

}    // namespace coContext

template<>
constexpr auto std::swap(coContext::Log &lhs, coContext::Log &rhs) noexcept -> void {
    lhs.swap(rhs);
}

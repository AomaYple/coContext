#pragma once

#include "Exception.hpp"

#include <sys/eventfd.h>

namespace coContext::internal {
    class Logger {
    public:
        constexpr Logger() = default;

        Logger(const Logger &) = delete;

        auto operator=(const Logger &) -> Logger & = delete;

        Logger(Logger &&) noexcept;

        auto operator=(Logger &&) noexcept -> Logger &;

        ~Logger();

        auto swap(Logger &other) noexcept -> void;

        auto setLevel(Log::Level level) noexcept -> void;

        [[nodiscard]] auto getFileDescriptor() const noexcept -> std::int32_t;

        [[nodiscard]] auto read() -> std::pmr::vector<std::byte>;

        auto write(Log log, std::source_location sourceLocation = std::source_location::current()) -> void;

    private:
        Log::Level level{Log::Level::info};
        std::pmr::vector<Log> logs{getMemoryResource()};
        std::int32_t fileDescriptor{[](const std::source_location sourceLocation = std::source_location::current()) {
            const std::int32_t result{eventfd(0, 0)};
            if (result == -1) {
                throw Exception{
                    Log{Log::Level::fatal,
                        std::pmr::string{std::error_code{errno, std::generic_category()}.message(),
                                         getMemoryResource()},
                        sourceLocation}
                };
            }

            return result;
        }()};
    };
}    // namespace coContext::internal

template<>
constexpr auto std::swap(coContext::internal::Logger &lhs, coContext::internal::Logger &rhs) noexcept -> void {
    lhs.swap(rhs);
}

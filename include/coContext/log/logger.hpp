#pragma once

#include "Log.hpp"

namespace coContext::logger {
    auto run() -> void;

    auto stop() noexcept -> void;

    auto setOutputStream(std::ostream *outputStream) noexcept -> void;

    auto enableWrite() noexcept -> void;

    auto disableWrite() noexcept -> void;

    [[nodiscard]] auto getLevel() noexcept -> Log::Level;

    auto setLevel(Log::Level level) noexcept -> void;

    auto write(Log log) -> void;

    auto flush() -> void;

}    // namespace coContext::logger

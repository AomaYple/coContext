#pragma once

#include "Log.hpp"

namespace coContext::logger {
    auto run() -> void;

    auto stop() noexcept -> void;

    auto setOutputStream(std::ostream *newOutputStream) noexcept -> void;

    auto enableWrite() noexcept -> void;

    auto disableWrite() noexcept -> void;

    [[nodiscard]] auto getLevel() noexcept -> Log::Level;

    auto setLevel(Log::Level newLevel) noexcept -> void;

    auto write(Log log) -> void;

    auto flush() -> void;

}    // namespace coContext::logger

#pragma once

#include "Log.hpp"

namespace coContext::logger {
    auto run() -> void;

    auto stop() -> void;

    auto setOutputStream(std::ostream *outputStream) -> void;

    auto enableWrite() -> void;

    auto disableWrite() -> void;

    [[nodiscard]] auto getLevel() -> Log::Level;

    auto setLevel(Log::Level level) -> void;

    auto write(Log log) -> void;

    auto flush() -> void;

}    // namespace coContext::logger

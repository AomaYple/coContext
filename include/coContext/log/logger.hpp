#pragma once

#include "Log.hpp"

namespace coContext {
    auto onLogging() -> void;

    auto offLogging() noexcept -> void;

    auto startLogging() -> void;

    auto stopLogging() noexcept -> void;

    auto setLevel(Log::Level level) noexcept -> void;

    auto writeLog(Log log) -> void;
}    // namespace coContext

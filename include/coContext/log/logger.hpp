#pragma once

#include "Log.hpp"

namespace coContext {
    auto startLogging() -> void;

    auto stopLogging() noexcept -> void;

    auto setLevel(Log::Level level) noexcept -> void;

    auto enableWriteLog() noexcept -> void;

    auto disableWriteLog() noexcept -> void;

    auto writeLog(Log log) -> void;

    auto setOutputStream(std::ostream &outStream) noexcept -> void;
}    // namespace coContext

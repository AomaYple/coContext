#pragma once

#include "coroutine/Task.hpp"

namespace coContext {
    [[noreturn]] auto run() -> void;

    auto spawn(Task &&task) -> void;
}    // namespace coContext

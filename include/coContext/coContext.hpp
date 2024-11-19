#pragma once

#include "coroutine/AsyncWaiter.hpp"

namespace coContext {
    [[noreturn]] auto run() -> void;

    auto spawn(Task &&task) -> void;

    [[nodiscard]] auto close(std::int32_t fileDescriptor) -> AsyncWaiter;
}    // namespace coContext

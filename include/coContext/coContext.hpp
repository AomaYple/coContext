#pragma once

namespace coContext {
    class Task;

    [[noreturn]] auto run() -> void;

    auto spawn(Task &&task) -> void;
}    // namespace coContext

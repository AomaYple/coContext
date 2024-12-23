#pragma once

#include <memory_resource>

namespace coContext {
    [[nodiscard]] auto getMemoryResource() -> std::pmr::memory_resource *;
}    // namespace coContext

#pragma once

#include <memory_resource>

namespace coContext::internal {
    [[nodiscard]] auto getSyncMemoryResource() -> std::pmr::memory_resource *;

    [[nodiscard]] auto getUnsyncMemoryResource() -> std::pmr::memory_resource *;
}    // namespace coContext::internal

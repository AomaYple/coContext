#pragma once

#include <memory_resource>

namespace coContext::internal {
    [[nodiscard]] auto getUnSyncMemoryResource() -> std::pmr::memory_resource *;

    [[nodiscard]] auto getSyncMemoryResource() -> std::pmr::memory_resource *;
}    // namespace coContext::internal

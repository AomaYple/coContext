#pragma once

#include <memory_resource>

namespace coContext::internal {
    [[nodiscard]] auto getMemoryResource() -> std::pmr::memory_resource *;
}    // namespace coContext::internal

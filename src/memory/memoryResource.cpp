#include "coContext/memory/memoryResource.hpp"

auto coContext::getMemoryResource() -> std::pmr::memory_resource * {
    thread_local std::pmr::unsynchronized_pool_resource resource;

    return std::addressof(resource);
}

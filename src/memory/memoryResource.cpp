#include "coContext/memory/memoryResource.hpp"

#include "MiMallocResource.hpp"

auto coContext::getMemoryResource() -> std::pmr::memory_resource * {
    static constinit MiMallocResource miMallocResource;
    thread_local std::pmr::unsynchronized_pool_resource resource{std::addressof(miMallocResource)};

    return std::addressof(resource);
}

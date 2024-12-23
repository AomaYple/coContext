#include "memoryResource.hpp"

#include "MiMallocResource.hpp"

auto coContext::getMemoryResource() -> std::pmr::memory_resource * {
#ifndef NDEBUG
    thread_local std::pmr::unsynchronized_pool_resource resource;
#else
    static constinit MiMallocResource miMallocResource;
    thread_local std::pmr::unsynchronized_pool_resource resource{std::addressof(miMallocResource)};
#endif

    return std::addressof(resource);
}

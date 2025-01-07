#include "coContext/memory/memoryResource.hpp"

#ifdef NDEBUG
    #include "MiMallocResource.hpp"

namespace {
    constinit coContext::internal::MiMallocResource miMallocResource;
}    // namespace
#endif    // NDEBUG

auto coContext::internal::getUnSyncMemoryResource() -> std::pmr::memory_resource * {
#ifndef NDEBUG
    thread_local std::pmr::unsynchronized_pool_resource resource;
#else     // NDEBUG
    thread_local std::pmr::unsynchronized_pool_resource resource{std::addressof(miMallocResource)};
#endif    // NDEBUG

    return std::addressof(resource);
}

auto coContext::internal::getSyncMemoryResource() -> std::pmr::memory_resource * {
#ifndef NDEBUG
    static std::pmr::synchronized_pool_resource resource;
#else     // NDEBUG
    static std::pmr::synchronized_pool_resource resource{std::addressof(miMallocResource)};
#endif    // NDEBUG

    return std::addressof(resource);
}

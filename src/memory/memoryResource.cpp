#include "coContext/memory/memoryResource.hpp"

#ifdef NDEBUG
    #include "MiMallocResource.hpp"

namespace {
    [[nodiscard]] constexpr auto getMiMallocResource() noexcept {
        static constinit coContext::internal::MiMallocResource miMallocResource;

        return std::addressof(miMallocResource);
    }
}    // namespace
#endif    // NDEBUG

auto coContext::internal::getSyncMemoryResource() -> std::pmr::memory_resource * {
#ifdef NDEBUG
    static std::pmr::synchronized_pool_resource resource{getMiMallocResource()};
#else     // NDEBUG
    static std::pmr::synchronized_pool_resource resource;
#endif    // NDEBUG

    return std::addressof(resource);
}

auto coContext::internal::getUnSyncMemoryResource() -> std::pmr::memory_resource * {
#ifdef NDEBUG
    thread_local std::pmr::unsynchronized_pool_resource resource{getMiMallocResource()};
#else     // NDEBUG
    thread_local std::pmr::unsynchronized_pool_resource resource;
#endif    // NDEBUG

    return std::addressof(resource);
}

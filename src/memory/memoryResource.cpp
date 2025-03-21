#include "coContext/memory/memoryResource.hpp"

#ifdef NDEBUG
    #include "MiMallocResource.hpp"

namespace {
    [[nodiscard]] constexpr auto getUpstreamResource() noexcept {
        static constinit coContext::internal::MiMallocResource resource;

        return std::addressof(resource);
    }
}    // namespace
#endif    // NDEBUG

auto coContext::internal::getSyncMemoryResource() -> std::pmr::memory_resource * {
#ifdef NDEBUG
    static std::pmr::synchronized_pool_resource resource{getUpstreamResource()};
#else     // NDEBUG
    static std::pmr::synchronized_pool_resource resource;
#endif    // NDEBUG

    return std::addressof(resource);
}

auto coContext::internal::getUnsyncMemoryResource() -> std::pmr::memory_resource * {
#ifdef NDEBUG
    thread_local std::pmr::unsynchronized_pool_resource resource{getUpstreamResource()};
#else     // NDEBUG
    thread_local std::pmr::unsynchronized_pool_resource resource;
#endif    // NDEBUG

    return std::addressof(resource);
}

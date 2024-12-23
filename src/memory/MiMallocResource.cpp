#include "MiMallocResource.hpp"

#include <mimalloc.h>

auto coContext::internal::MiMallocResource::do_allocate(const std::size_t numberOfBytes, const std::size_t alignment)
    -> void * {
    return mi_new_aligned(numberOfBytes, alignment);
}

auto coContext::internal::MiMallocResource::do_deallocate(void *const pointer, const std::size_t numberOfBytes,
                                                          const std::size_t alignment) noexcept -> void {
    mi_free_size_aligned(pointer, numberOfBytes, alignment);
}

auto coContext::internal::MiMallocResource::do_is_equal(const memory_resource &other) const noexcept -> bool {
    return this == std::addressof(other);
}

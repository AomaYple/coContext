#include "BufferGroup.hpp"

#include "../memory/memoryResource.hpp"

coContext::internal::BufferGroup::BufferGroup(const std::size_t bufferCount, const std::size_t bufferSize) :
    buffers{bufferCount, getMemoryResource()}, bufferSize{bufferSize} {}

auto coContext::internal::BufferGroup::swap(BufferGroup &other) noexcept -> void {
    std::swap(this->buffers, other.buffers);
    std::swap(this->bufferSize, other.bufferSize);
}

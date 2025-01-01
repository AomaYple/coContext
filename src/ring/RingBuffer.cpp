#include "RingBuffer.hpp"

#include "Ring.hpp"

#include <utility>

coContext::internal::RingBuffer::RingBuffer(std::shared_ptr<Ring> ring, const std::uint32_t entries,
                                            const std::int32_t id, const std::uint32_t flags) :
    ring{std::move(ring)}, handle{this->ring->setupRingBuffer(entries, id, flags)}, entries{entries}, id{id} {}

coContext::internal::RingBuffer::RingBuffer(RingBuffer &&other) noexcept :
    ring{std::move(other.ring)}, handle{std::exchange(other.handle, nullptr)}, entries{other.entries}, id{other.id},
    offset{other.offset} {}

auto coContext::internal::RingBuffer::operator=(RingBuffer &&other) noexcept -> RingBuffer & {
    if (this == std::addressof(other)) return *this;

    this->~RingBuffer();

    this->ring = std::move(other.ring);
    this->handle = std::exchange(other.handle, nullptr);
    this->entries = other.entries;
    this->id = other.id;
    this->offset = other.offset;

    return *this;
}

coContext::internal::RingBuffer::~RingBuffer() {
    if (this->handle != nullptr) this->ring->freeRingBuffer(this->handle, this->entries, this->id);
}

auto coContext::internal::RingBuffer::swap(RingBuffer &other) noexcept -> void {
    std::swap(this->ring, other.ring);
    std::swap(this->handle, other.handle);
    std::swap(this->entries, other.entries);
    std::swap(this->id, other.id);
    std::swap(this->offset, other.offset);
}

auto coContext::internal::RingBuffer::addBuffer(const std::span<std::byte> buffer, const std::uint16_t index) noexcept
    -> void {
    io_uring_buf_ring_add(this->handle, std::data(buffer), std::size(buffer), index,
                          io_uring_buf_ring_mask(this->entries), this->offset++);
}

auto coContext::internal::RingBuffer::advance(const std::int32_t count) noexcept -> void {
    this->ring->advance(this->handle, count, std::exchange(this->offset, 0));
}

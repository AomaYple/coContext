#include "RingBuffer.hpp"

#include "Ring.hpp"

#include <utility>

coContext::internal::RingBuffer::RingBuffer(std::shared_ptr<Ring> ring, const std::uint32_t entries,
                                            const std::int32_t id, const std::uint32_t flags) :
    ring{std::move(ring)}, handle{this->ring->setupRingBuffer(entries, id, flags)}, entries{entries}, id{id} {}

coContext::internal::RingBuffer::RingBuffer(RingBuffer &&other) noexcept :
    buffers{std::move(other.buffers)}, ring{std::move(other.ring)}, handle{std::exchange(other.handle, nullptr)},
    entries{other.entries}, id{other.id}, offset{other.offset} {}

auto coContext::internal::RingBuffer::operator=(RingBuffer &&other) noexcept -> RingBuffer & {
    if (this == std::addressof(other)) return *this;

    this->~RingBuffer();

    this->buffers = std::move(other.buffers);
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
    std::swap(this->buffers, other.buffers);
    std::swap(this->ring, other.ring);
    std::swap(this->handle, other.handle);
    std::swap(this->entries, other.entries);
    std::swap(this->id, other.id);
    std::swap(this->offset, other.offset);
}

auto coContext::internal::RingBuffer::getId() const noexcept -> std::int32_t { return this->id; }

auto coContext::internal::RingBuffer::advance(const std::int32_t count) noexcept -> void {
    this->ring->advance(this->handle, count, std::exchange(this->offset, 0));
}

auto coContext::internal::RingBuffer::readData(const std::uint16_t bufferId, const std::size_t dataSize) noexcept
    -> std::span<const std::byte> {
    auto &[data, offset]{this->buffers[bufferId]};
    this->addBuffer(bufferId);

    const std::span subData{std::cbegin(data) + static_cast<std::ptrdiff_t>(offset), dataSize};
    offset += dataSize;

    return subData;
}

auto coContext::internal::RingBuffer::markBufferUsed(const std::uint16_t bufferId) noexcept -> void {
    this->buffers[bufferId].offset = 0;
}

auto coContext::internal::RingBuffer::expandBuffer() -> void {
    if (std::size(this->buffers) == this->entries) return;

    this->buffers.emplace_back();
    this->addBuffer(std::size(this->buffers) - 1);
}

auto coContext::internal::RingBuffer::addBuffer(const std::uint16_t bufferId) noexcept -> void {
    io_uring_buf_ring_add(this->handle, std::data(this->buffers[bufferId].data),
                          std::size(this->buffers[bufferId].data), bufferId, io_uring_buf_ring_mask(this->entries),
                          this->offset++);
}

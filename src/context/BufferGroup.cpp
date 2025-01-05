#include "BufferGroup.hpp"

#include "../ring/RingBuffer.hpp"

coContext::internal::BufferGroup::BufferGroup(std::shared_ptr<RingBuffer> ringBuffer) :
    ringBuffer{std::move(ringBuffer)} {}

auto coContext::internal::BufferGroup::swap(BufferGroup &other) noexcept -> void {
    std::swap(this->ringBuffer, other.ringBuffer);
    std::swap(this->buffers, other.buffers);
}

auto coContext::internal::BufferGroup::getId() const noexcept -> std::int32_t { return this->ringBuffer->getId(); }

auto coContext::internal::BufferGroup::readFromBuffer(const std::uint16_t bufferId, const std::size_t dataSize) noexcept
    -> std::span<const std::byte> {
    auto &[buffer, offset]{this->buffers[bufferId]};
    this->ringBuffer->addBuffer(buffer, bufferId);

    const std::span data{std::cbegin(buffer) + static_cast<std::ptrdiff_t>(offset), dataSize};
    offset += dataSize;

    return data;
}

auto coContext::internal::BufferGroup::markBufferUsed(const std::uint16_t bufferId) noexcept -> void {
    this->buffers[bufferId].offset = 0;
}

auto coContext::internal::BufferGroup::expandBuffer() -> void {
    if (std::size(this->buffers) == this->ringBuffer->getEntries()) return;

    this->buffers.emplace_back();
    this->ringBuffer->addBuffer(this->buffers.back().buffer, std::size(this->buffers) - 1);
}

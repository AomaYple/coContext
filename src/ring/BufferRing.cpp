#include "BufferRing.hpp"

#include "Ring.hpp"
#include "coContext/log/logger.hpp"

#include <utility>

using namespace std::string_view_literals;

coContext::internal::BufferRing::BufferRing(std::shared_ptr<Ring> ring, const std::uint32_t entries,
                                            const std::int32_t id, const std::uint32_t flags) :
    ring{std::move(ring)}, handle{this->ring->setupBufferRing(entries, id, flags)}, entries{entries}, id{id} {}

coContext::internal::BufferRing::BufferRing(BufferRing &&other) noexcept :
    buffers{std::move(other.buffers)}, ring{std::move(other.ring)}, handle{std::exchange(other.handle, nullptr)},
    entries{other.entries}, id{other.id}, offset{other.offset} {}

auto coContext::internal::BufferRing::operator=(BufferRing &&other) noexcept -> BufferRing & {
    if (this == std::addressof(other)) return *this;

    this->~BufferRing();

    this->buffers = std::move(other.buffers);
    this->ring = std::move(other.ring);
    this->handle = std::exchange(other.handle, nullptr);
    this->entries = other.entries;
    this->id = other.id;
    this->offset = other.offset;

    return *this;
}

coContext::internal::BufferRing::~BufferRing() {
    if (this->handle != nullptr) this->ring->freeBufferRing(this->handle, this->entries, this->id);
}

auto coContext::internal::BufferRing::swap(BufferRing &other) noexcept -> void {
    std::swap(this->buffers, other.buffers);
    std::swap(this->ring, other.ring);
    std::swap(this->handle, other.handle);
    std::swap(this->entries, other.entries);
    std::swap(this->id, other.id);
    std::swap(this->offset, other.offset);
}

auto coContext::internal::BufferRing::getId() const noexcept -> std::int32_t { return this->id; }

auto coContext::internal::BufferRing::advance(const std::int32_t count) noexcept -> void {
    this->ring->advance(this->handle, count, std::exchange(this->offset, 0));
}

auto coContext::internal::BufferRing::readData(const std::uint16_t bufferId, const std::size_t dataSize) noexcept
    -> std::span<const std::byte> {
    this->addBuffer(bufferId);

    auto &[data, offset]{this->buffers[bufferId]};
    const std::span subData{std::cbegin(data) + static_cast<std::ptrdiff_t>(offset), dataSize};
    offset += dataSize;

    return subData;
}

auto coContext::internal::BufferRing::markBufferUsed(const std::uint16_t bufferId) noexcept -> void {
    this->buffers[bufferId].offset = 0;
}

auto coContext::internal::BufferRing::expandBuffer(const std::source_location sourceLocation) -> void {
    if (std::size(this->buffers) == this->entries) {
        logger::write(Log{
            Log::Level::warn, std::pmr::string{"number of buffer has reached the limit"sv, getSyncMemoryResource()},
            sourceLocation
        });

        return;
    }

    this->buffers.emplace_back();
    this->addBuffer(std::size(this->buffers) - 1);
}

auto coContext::internal::BufferRing::addBuffer(const std::uint16_t bufferId) noexcept -> void {
    const std::span data{this->buffers[bufferId].data};
    io_uring_buf_ring_add(this->handle, std::data(data), std::size(data), bufferId,
                          io_uring_buf_ring_mask(this->entries), this->offset++);
}

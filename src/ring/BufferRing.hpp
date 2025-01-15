#pragma once

#include "coContext/memory/memoryResource.hpp"

#include <liburing/io_uring.h>
#include <memory>
#include <source_location>

namespace coContext::internal {
    class Ring;

    class BufferRing {
        struct Buffer {
            std::pmr::vector<std::byte> data{1024, getUnSyncMemoryResource()};
            std::size_t offset{};
        };

    public:
        BufferRing(std::shared_ptr<Ring> ring, std::uint32_t entries, std::int32_t id, std::uint32_t flags);

        BufferRing(const BufferRing &) = delete;

        auto operator=(const BufferRing &) -> BufferRing & = delete;

        BufferRing(BufferRing &&) noexcept;

        auto operator=(BufferRing &&) noexcept -> BufferRing &;

        ~BufferRing();

        auto swap(BufferRing &other) noexcept -> void;

        [[nodiscard]] auto getId() const noexcept -> std::int32_t;

        auto advance(std::int32_t count) noexcept -> void;

        [[nodiscard]] auto readData(std::uint16_t bufferId, std::size_t dataSize) noexcept
            -> std::span<const std::byte>;

        auto markBufferUsed(std::uint16_t bufferId) noexcept -> void;

        auto expandBuffer(std::source_location sourceLocation = std::source_location::current()) -> void;

    private:
        auto addBuffer(std::uint16_t bufferId) noexcept -> void;

        std::pmr::vector<Buffer> buffers{getUnSyncMemoryResource()};
        std::shared_ptr<Ring> ring;
        io_uring_buf_ring *handle;
        std::uint32_t entries;
        std::int32_t id, offset{};
    };
}    // namespace coContext::internal

template<>
constexpr auto std::swap(coContext::internal::BufferRing &lhs, coContext::internal::BufferRing &rhs) noexcept -> void {
    lhs.swap(rhs);
}

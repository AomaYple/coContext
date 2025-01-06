#pragma once

#include "coContext/memory/memoryResource.hpp"

#include <liburing/io_uring.h>
#include <memory>

namespace coContext::internal {
    class Ring;

    class RingBuffer {
        struct Buffer {
            std::pmr::vector<std::byte> data{1024, getMemoryResource()};
            std::size_t offset{};
        };

    public:
        RingBuffer(std::shared_ptr<Ring> ring, std::uint32_t entries, std::int32_t id, std::uint32_t flags);

        RingBuffer(const RingBuffer &) = delete;

        auto operator=(const RingBuffer &) -> RingBuffer & = delete;

        RingBuffer(RingBuffer &&) noexcept;

        auto operator=(RingBuffer &&) noexcept -> RingBuffer &;

        ~RingBuffer();

        auto swap(RingBuffer &other) noexcept -> void;

        [[nodiscard]] auto getId() const noexcept -> std::int32_t;

        auto advance(std::int32_t count) noexcept -> void;

        [[nodiscard]] auto readData(std::uint16_t bufferId, std::size_t dataSize) noexcept
            -> std::span<const std::byte>;

        auto markBufferUsed(std::uint16_t bufferId) noexcept -> void;

        auto expandBuffer() -> void;

    private:
        auto addBuffer(std::uint16_t bufferId) noexcept -> void;

        std::pmr::vector<Buffer> buffers{getMemoryResource()};
        std::shared_ptr<Ring> ring;
        io_uring_buf_ring *handle;
        std::uint32_t entries;
        std::int32_t id, offset{};
    };
}    // namespace coContext::internal

template<>
constexpr auto std::swap(coContext::internal::RingBuffer &lhs, coContext::internal::RingBuffer &rhs) noexcept -> void {
    lhs.swap(rhs);
}

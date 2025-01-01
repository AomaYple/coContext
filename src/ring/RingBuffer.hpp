#pragma once

#include <liburing/io_uring.h>
#include <memory>

namespace coContext::internal {
    class Ring;

    class RingBuffer {
    public:
        RingBuffer(std::shared_ptr<Ring> ring, std::uint32_t entries, std::int32_t id, std::uint32_t flags);

        RingBuffer(const RingBuffer &) = delete;

        auto operator=(const RingBuffer &) -> RingBuffer & = delete;

        RingBuffer(RingBuffer &&) noexcept;

        auto operator=(RingBuffer &&) noexcept -> RingBuffer &;

        ~RingBuffer();

        auto swap(RingBuffer &other) noexcept -> void;

        auto addBuffer(std::span<std::byte> buffer, std::uint16_t index) noexcept -> void;

        auto advance(std::int32_t count) noexcept -> void;

    private:
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

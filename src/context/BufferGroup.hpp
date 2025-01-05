#pragma once

#include "../memory/memoryResource.hpp"

#include <memory>

namespace coContext::internal {
    class RingBuffer;

    class BufferGroup {
        struct Buffer {
            std::pmr::vector<std::byte> buffer{1024, getMemoryResource()};
            std::size_t offset{};
        };

    public:
        explicit BufferGroup(std::shared_ptr<RingBuffer> ringBuffer);

        BufferGroup(const BufferGroup &) = delete;

        auto operator=(const BufferGroup &) -> BufferGroup & = delete;

        BufferGroup(BufferGroup &&) noexcept = default;

        auto operator=(BufferGroup &&) noexcept -> BufferGroup & = default;

        ~BufferGroup() = default;

        auto swap(BufferGroup &other) noexcept -> void;

        [[nodiscard]] auto getId() const noexcept -> std::int32_t;

        [[nodiscard]] auto readFromBuffer(std::uint16_t bufferId, std::size_t dataSize) noexcept
            -> std::span<const std::byte>;

        auto markBufferUsed(std::uint16_t bufferId) noexcept -> void;

        auto expandBuffer() -> void;

    private:
        std::shared_ptr<RingBuffer> ringBuffer;
        std::pmr::vector<Buffer> buffers{getMemoryResource()};
    };
}    // namespace coContext::internal

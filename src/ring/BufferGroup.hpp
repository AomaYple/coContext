#pragma once

#include <vector>

namespace coContext::internal {
    class BufferGroup {
    public:
        BufferGroup(std::size_t bufferCount, std::size_t bufferSize);

        auto swap(BufferGroup &other) noexcept -> void;

    private:
        std::pmr::vector<std::pmr::vector<std::byte>> buffers;
        std::size_t bufferSize;
    };
}    // namespace coContext::internal

template<>
constexpr auto std::swap(coContext::internal::BufferGroup &lhs, coContext::internal::BufferGroup &rhs) noexcept
    -> void {
    lhs.swap(rhs);
}

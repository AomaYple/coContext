#pragma once

#include <functional>
#include <liburing.h>
#include <source_location>

namespace coContext::internal {
    class Completion;

    class Ring {
    public:
        Ring(std::uint32_t entries, io_uring_params *parameters);

        Ring(const Ring &) = delete;

        auto operator=(const Ring &) -> Ring & = delete;

        Ring(Ring &&) noexcept;

        auto operator=(Ring &&) noexcept -> Ring &;

        ~Ring();

        auto swap(Ring &other) noexcept -> void;

        auto registerSelfFileDescriptor(std::source_location sourceLocation = std::source_location::current()) -> void;

        auto registerSparseFileDescriptor(std::uint32_t count,
                                          std::source_location sourceLocation = std::source_location::current())
            -> void;

        [[nodiscard]] auto setupBufferRing(std::uint32_t entries, std::int32_t id, std::uint32_t flags,
                                           std::source_location sourceLocation = std::source_location::current())
            -> io_uring_buf_ring *;

        auto freeBufferRing(io_uring_buf_ring *bufferRing, std::uint32_t entries, std::int32_t id,
                            std::source_location sourceLocation = std::source_location::current()) -> void;

        [[nodiscard]] auto getSubmission(std::source_location sourceLocation = std::source_location::current())
            -> io_uring_sqe *;

        auto submit(std::source_location sourceLocation = std::source_location::current()) -> void;

        auto submitAndWait(std::uint32_t count, std::source_location sourceLocation = std::source_location::current())
            -> void;

        [[nodiscard]] auto poll(std::move_only_function<auto(Completion)->void> action) const -> std::int32_t;

        auto advance(io_uring_buf_ring *bufferRing, std::int32_t completionCount, std::int32_t bufferCount) noexcept
            -> void;

        [[nodiscard]] auto syncCancel(io_uring_sync_cancel_reg *parameters,
                                      std::source_location sourceLocation = std::source_location::current())
            -> std::int32_t;

    private:
        io_uring handle;
    };
}    // namespace coContext::internal

template<>
constexpr auto std::swap(coContext::internal::Ring &lhs, coContext::internal::Ring &rhs) noexcept -> void {
    lhs.swap(rhs);
}

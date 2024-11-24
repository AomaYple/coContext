#pragma once

#include <functional>
#include <liburing.h>
#include <source_location>
#include <span>

namespace coContext {
    class Ring {
    public:
        Ring(std::uint32_t entries, io_uring_params &params);

        Ring(const Ring &) = delete;

        auto operator=(const Ring &) -> Ring & = delete;

        Ring(Ring &&) noexcept;

        auto operator=(Ring &&) noexcept -> Ring &;

        ~Ring();

        auto swap(Ring &other) noexcept -> void;

        [[nodiscard]] auto getFileDescriptor() const noexcept -> std::int32_t;

        auto registerSelfFileDescriptor(std::source_location sourceLocation = std::source_location::current()) -> void;

        auto registerCpuAffinity(std::size_t cpuSetSize, const cpu_set_t *cpuSet,
                                 std::source_location sourceLocation = std::source_location::current()) -> void;

        auto registerSparseFileDescriptor(std::uint32_t count,
                                          std::source_location sourceLocation = std::source_location::current())
            -> void;

        auto allocateFileDescriptorRange(std::uint32_t offset, std::uint32_t length,
                                         std::source_location sourceLocation = std::source_location::current()) -> void;

        auto updateFileDescriptors(std::uint32_t offset, std::span<const std::int32_t> fileDescriptors,
                                   std::source_location sourceLocation = std::source_location::current()) -> void;

        [[nodiscard]] auto setupRingBuffer(std::uint32_t entries, std::int32_t id, std::uint32_t flags,
                                           std::source_location sourceLocation = std::source_location::current())
            -> io_uring_buf_ring *;

        auto freeRingBuffer(io_uring_buf_ring *ringBuffer, std::uint32_t entries, std::int32_t id,
                            std::source_location sourceLocation = std::source_location::current()) -> void;

        [[nodiscard]] auto syncCancel(io_uring_sync_cancel_reg &parameters,
                                      std::source_location sourceLocation = std::source_location::current())
            -> std::int32_t;

        [[nodiscard]] auto
            getSubmissionQueueEntry(std::source_location sourceLocation = std::source_location::current())
                -> io_uring_sqe *;

        auto submitAndWait(std::uint32_t count, std::source_location sourceLocation = std::source_location::current())
            -> void;

        [[nodiscard]] auto poll(std::move_only_function<auto(const io_uring_cqe *)->void> &&action) const
            -> std::int32_t;

        auto advance(std::uint32_t count) noexcept -> void;

        auto advance(io_uring_buf_ring *ringBuffer, std::int32_t completionQueueEntry,
                     std::int32_t bufferCount) noexcept -> void;

    private:
        auto destroy() noexcept -> void;

        io_uring handle;
    };
}    // namespace coContext

template<>
constexpr auto std::swap(coContext::Ring &lhs, coContext::Ring &rhs) noexcept -> void {
    lhs.swap(rhs);
}

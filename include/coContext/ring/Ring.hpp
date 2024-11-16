#pragma once

#include <functional>
#include <liburing.h>
#include <source_location>
#include <span>

namespace coContext {
    class Ring {
    public:
        Ring(unsigned int entries, io_uring_params &params);

        Ring(const Ring &) = delete;

        auto operator=(const Ring &) -> Ring & = delete;

        Ring(Ring &&) noexcept;

        auto operator=(Ring &&) noexcept -> Ring &;

        ~Ring();

        [[nodiscard]] auto getFileDescriptor() const noexcept -> int;

        auto registerSelfFileDescriptor(std::source_location sourceLocation = std::source_location::current()) -> void;

        auto registerCpuAffinity(unsigned long cpuSetSize, const cpu_set_t *cpuSet,
                                 std::source_location sourceLocation = std::source_location::current()) -> void;

        auto registerSparseFileDescriptor(unsigned int count,
                                          std::source_location sourceLocation = std::source_location::current())
            -> void;

        auto allocateFileDescriptorRange(unsigned int offset, unsigned int length,
                                         std::source_location sourceLocation = std::source_location::current()) -> void;

        auto updateFileDescriptors(unsigned int offset, std::span<const int> fileDescriptors,
                                   std::source_location sourceLocation = std::source_location::current()) -> void;

        [[nodiscard]] auto setupRingBuffer(unsigned int entries, int id, unsigned int flags,
                                           std::source_location sourceLocation = std::source_location::current())
            -> io_uring_buf_ring *;

        auto freeRingBuffer(io_uring_buf_ring *ringBuffer, unsigned int entries, int id,
                            std::source_location sourceLocation = std::source_location::current()) -> void;

        [[nodiscard]] auto getSqe(std::source_location sourceLocation = std::source_location::current())
            -> io_uring_sqe *;

        auto submitAndWait(unsigned int count, std::source_location sourceLocation = std::source_location::current())
            -> void;

        auto poll(std::move_only_function<auto(const io_uring_cqe *)->void> &&action) const -> int;

        auto advance(unsigned int count) noexcept -> void;

        auto advance(io_uring_buf_ring *ringBuffer, int cqeCount, int bufferCount) noexcept -> void;

    private:
        auto destroy() noexcept -> void;

        io_uring handle;
    };
}    // namespace coContext

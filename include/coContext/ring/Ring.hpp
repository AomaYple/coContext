#pragma once

#include <functional>
#include <liburing.h>
#include <source_location>

namespace coContext {
    class Ring {
    public:
        Ring(unsigned int entries, io_uring_params &params);

        Ring(const Ring &) = delete;

        Ring(Ring &&) noexcept;

        auto operator=(const Ring &) -> Ring & = delete;

        auto operator=(Ring &&) noexcept -> Ring &;

        ~Ring();

        [[nodiscard]] auto getFileDescriptor() const noexcept -> int;

        auto registerSelfFileDescriptor(std::source_location sourceLocation = std::source_location::current()) -> void;

        auto registerCpuAffinity(const cpu_set_t &cpuSet,
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

#pragma once

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

    private:
        auto destroy() noexcept -> void;

        io_uring handle;
    };
}    // namespace coContext

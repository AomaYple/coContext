#pragma once

#include <liburing.h>

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

    private:
        auto destroy() noexcept -> void;

        io_uring handle;
    };
}    // namespace coContext

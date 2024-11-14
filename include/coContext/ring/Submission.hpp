#pragma once

#include <liburing.h>
#include <variant>

namespace coContext {
    class Submission {
    public:
        enum class Type : unsigned char { close };

        struct Close {};

        using parameterType = std::variant<Close>;

        Submission(int fileDescriptor, unsigned int flags, unsigned short ioPriority,
                   parameterType &&parameter) noexcept;

        auto setUserData(unsigned long userData) noexcept -> void;

        auto setSqe(io_uring_sqe *sqe) const noexcept -> void;

    private:
        int fileDescriptor;
        unsigned int flags;
        unsigned short ioPriority;
        unsigned long userData;
        parameterType parameter;
    };
}    // namespace coContext

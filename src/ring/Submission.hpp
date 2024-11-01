#pragma once

#include <variant>

namespace coContext {
    struct Submission {
        enum class Type : unsigned char { close };

        struct Close {};

        int fileDescriptor;
        unsigned int flags;
        unsigned short ioPriority;
        unsigned long userData;
        std::variant<Close> parameter;
    };
}    // namespace coContext

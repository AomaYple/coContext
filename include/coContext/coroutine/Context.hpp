#pragma once

#include "coContext/ring/Ring.hpp"

namespace coContext {
    class Context {
    public:
        Context();

        Context(const Context &) = delete;

        Context(Context &&) noexcept = default;

        auto operator=(const Context &) -> Context & = delete;

        auto operator=(Context &&) noexcept -> Context & = default;

        ~Context() = default;

    private:
        static auto getFileDescriptorLimit(std::source_location sourceLocation = std::source_location::current())
            -> unsigned long;

        static std::mutex mutex;
        static int sharedRingFileDescriptor;
        static unsigned int cpuCode;

        Ring ring;
    };
}    // namespace coContext

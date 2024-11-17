#pragma once

#include "Task.hpp"
#include "coContext/ring/Ring.hpp"

namespace coContext {
    class Context {
    public:
        Context();

        Context(const Context &) = delete;

        auto operator=(const Context &) -> Context & = delete;

        Context(Context &&) noexcept = default;

        auto operator=(Context &&) noexcept -> Context & = default;

        ~Context() = default;

    private:
        static auto getFileDescriptorLimit(std::source_location sourceLocation = std::source_location::current())
            -> unsigned long;

        static std::mutex mutex;
        static int sharedRingFileDescriptor;
        static unsigned int cpuCode;

        Ring ring;
        std::unordered_map<unsigned long, Task> tasks;
    };
}    // namespace coContext

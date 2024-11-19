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

        [[noreturn]] auto run() -> void;

        [[nodiscard]] auto getSubmissionQueueEntry() -> io_uring_sqe *;

        auto submit(Task &&task) -> void;

    private:
        [[nodiscard]] static auto
            getFileDescriptorLimit(std::source_location sourceLocation = std::source_location::current())
                -> std::uint64_t;

        static std::mutex mutex;
        static std::int32_t sharedRingFileDescriptor;
        static std::uint32_t cpuCode;

        Ring ring;
        std::unordered_map<std::uint64_t, Task> tasks;
    };
}    // namespace coContext

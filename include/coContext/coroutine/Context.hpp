#pragma once

#include "coContext/ring/Ring.hpp"

namespace coContext {
    class AsyncWaiter;
    class Task;

    class Context {
    public:
        Context();

        Context(const Context &) = delete;

        auto operator=(const Context &) -> Context & = delete;

        Context(Context &&) noexcept = default;

        auto operator=(Context &&) noexcept -> Context & = default;

        ~Context() = default;

        [[noreturn]] auto run() -> void;

        auto submit(Task &&task) -> void;

        [[nodiscard]] auto close(std::int32_t fileDescriptor) -> AsyncWaiter;

        [[nodiscard]] auto socket(std::int32_t domain, std::int32_t type, std::int32_t protocol) -> AsyncWaiter;

        [[nodiscard]] auto bind(std::int32_t fileDescriptor, sockaddr *address, std::uint32_t addressLength)
            -> AsyncWaiter;

        [[nodiscard]] auto listen(std::int32_t fileDescriptor, std::int32_t backlog) -> AsyncWaiter;

        [[nodiscard]] auto accept(std::int32_t fileDescriptor, sockaddr *address, std::uint32_t *addressLength)
            -> AsyncWaiter;

        [[nodiscard]] auto accept4(std::int32_t fileDescriptor, sockaddr *address, std::uint32_t *addressLength,
                                   std::int32_t flags) -> AsyncWaiter;

        [[nodiscard]] auto connect(std::int32_t fileDescriptor, const sockaddr *address, std::uint32_t addressLength)
            -> AsyncWaiter;

        [[nodiscard]] auto shutdown(std::int32_t fileDescriptor, std::int32_t how) -> AsyncWaiter;

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

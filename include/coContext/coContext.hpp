#pragma once

#include "coroutine/AsyncWaiter.hpp"

#include <sys/socket.h>

namespace coContext {
    [[noreturn]] auto run() -> void;

    auto spawn(Task &&task) -> void;

    [[nodiscard]] auto close(std::int32_t fileDescriptor) -> AsyncWaiter;

    [[nodiscard]] auto socket(std::int32_t domain, std::int32_t type, std::int32_t protocol) -> AsyncWaiter;

    [[nodiscard]] auto bind(std::int32_t fileDescriptor, sockaddr *address, std::uint32_t addressLength) -> AsyncWaiter;

    [[nodiscard]] auto listen(std::int32_t fileDescriptor, std::int32_t backlog) -> AsyncWaiter;

    [[nodiscard]] auto accept(std::int32_t fileDescriptor, sockaddr *address, std::uint32_t *addressLength)
        -> AsyncWaiter;

    [[nodiscard]] auto accept4(std::int32_t fileDescriptor, sockaddr *address, std::uint32_t *addressLength,
                               std::int32_t flags) -> AsyncWaiter;

    [[nodiscard]] auto connect(std::int32_t fileDescriptor, const sockaddr *address, std::uint32_t addressLength)
        -> AsyncWaiter;

    [[nodiscard]] auto shutdown(std::int32_t fileDescriptor, std::int32_t how) -> AsyncWaiter;
}    // namespace coContext

#pragma once

#include "coroutine/AsyncWaiter.hpp"

#include <span>
#include <string_view>
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

    [[nodiscard]] auto recv(std::int32_t fileDescriptor, std::span<std::byte> buffer, std::int32_t flags)
        -> AsyncWaiter;

    [[nodiscard]] auto recvmsg(std::int32_t fileDescriptor, msghdr *message, std::uint32_t flags) -> AsyncWaiter;

    [[nodiscard]] auto send(std::int32_t fileDescriptor, std::span<const std::byte> buffer, std::int32_t flags)
        -> AsyncWaiter;

    [[nodiscard]] auto sendto(std::int32_t fileDescriptor, std::span<const std::byte> buffer, std::int32_t flags,
                              const sockaddr *address, std::uint32_t addressLength) -> AsyncWaiter;

    [[nodiscard]] auto sendmsg(std::int32_t fileDescriptor, const msghdr *message, std::uint32_t flags) -> AsyncWaiter;

    [[nodiscard]] auto open(std::string_view pathname, std::int32_t flags, std::uint32_t mode) -> AsyncWaiter;

    [[nodiscard]] auto openat(std::int32_t directoryFileDescriptor, std::string_view pathname, std::int32_t flags,
                              std::uint32_t mode) -> AsyncWaiter;
}    // namespace coContext

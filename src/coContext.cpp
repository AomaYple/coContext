#include "coContext/coContext.hpp"

#include "coContext/coroutine/Context.hpp"

static thread_local coContext::Context context;

auto coContext::run() -> void { context.run(); }

auto coContext::spawn(Task &&task) -> void { context.submit(std::move(task)); }

auto coContext::close(const std::int32_t fileDescriptor) -> AsyncWaiter { return context.close(fileDescriptor); }

auto coContext::socket(const std::int32_t domain, const std::int32_t type, const std::int32_t protocol) -> AsyncWaiter {
    return context.socket(domain, type, protocol);
}

auto coContext::bind(const std::int32_t fileDescriptor, sockaddr *const address, const std::uint32_t addressLength)
    -> AsyncWaiter {
    return context.bind(fileDescriptor, address, addressLength);
}

auto coContext::listen(const std::int32_t fileDescriptor, const std::int32_t backlog) -> AsyncWaiter {
    return context.listen(fileDescriptor, backlog);
}

auto coContext::accept(const std::int32_t fileDescriptor, sockaddr *const address, std::uint32_t *const addressLength)
    -> AsyncWaiter {
    return context.accept(fileDescriptor, address, addressLength, 0);
}

auto coContext::accept4(const std::int32_t fileDescriptor, sockaddr *const address, std::uint32_t *const addressLength,
                        const std::int32_t flags) -> AsyncWaiter {
    return context.accept(fileDescriptor, address, addressLength, flags);
}

auto coContext::connect(const std::int32_t fileDescriptor, const sockaddr *const address,
                        const std::uint32_t addressLength) -> AsyncWaiter {
    return context.connect(fileDescriptor, address, addressLength);
}

auto coContext::shutdown(const std::int32_t fileDescriptor, const std::int32_t how) -> AsyncWaiter {
    return context.shutdown(fileDescriptor, how);
}

auto coContext::recv(const std::int32_t fileDescriptor, const std::span<std::byte> buffer, const std::int32_t flags)
    -> AsyncWaiter {
    return context.recv(fileDescriptor, buffer, flags);
}

auto coContext::recvmsg(const std::int32_t fileDescriptor, msghdr *const message, const std::uint32_t flags)
    -> AsyncWaiter {
    return context.recvmsg(fileDescriptor, message, flags);
}

auto coContext::send(const std::int32_t fileDescriptor, const std::span<const std::byte> buffer,
                     const std::int32_t flags) -> AsyncWaiter {
    return context.send(fileDescriptor, buffer, flags);
}

auto coContext::sendto(const std::int32_t fileDescriptor, const std::span<const std::byte> buffer,
                       const std::int32_t flags, const sockaddr *const address, const std::uint32_t addressLength)
    -> AsyncWaiter {
    return context.sendto(fileDescriptor, buffer, flags, address, addressLength);
}

auto coContext::sendmsg(const std::int32_t fileDescriptor, const msghdr *const message, const std::uint32_t flags)
    -> AsyncWaiter {
    return context.sendmsg(fileDescriptor, message, flags);
}

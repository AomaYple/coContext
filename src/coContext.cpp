#include "coContext/coContext.hpp"

#include "coContext/coroutine/Context.hpp"

static thread_local coContext::Context context;

auto coContext::run() -> void { context.run(); }

auto coContext::spawn(Task &&task) -> void { context.submit(std::move(task)); }

auto coContext::close(const std::int32_t fileDescriptor) -> AsyncWaiter { return context.close(fileDescriptor); }

auto coContext::socket(const std::int32_t domain, const std::int32_t type, const std::int32_t protocol) -> AsyncWaiter {
    return context.socket(domain, type, protocol, 0);
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
    return context.receive(fileDescriptor, buffer, flags);
}

auto coContext::recvmsg(const std::int32_t fileDescriptor, msghdr *const message, const std::uint32_t flags)
    -> AsyncWaiter {
    return context.receive(fileDescriptor, message, flags);
}

auto coContext::send(const std::int32_t fileDescriptor, const std::span<const std::byte> buffer,
                     const std::int32_t flags) -> AsyncWaiter {
    return context.send(fileDescriptor, buffer, flags);
}

auto coContext::sendto(const std::int32_t fileDescriptor, const std::span<const std::byte> buffer,
                       const std::int32_t flags, const sockaddr *const address, const std::uint32_t addressLength)
    -> AsyncWaiter {
    return context.send(fileDescriptor, buffer, flags, address, addressLength);
}

auto coContext::sendmsg(const std::int32_t fileDescriptor, const msghdr *const message, const std::uint32_t flags)
    -> AsyncWaiter {
    return context.send(fileDescriptor, message, flags);
}

auto coContext::open(const std::string_view pathname, const std::int32_t flags, const std::uint32_t mode)
    -> AsyncWaiter {
    return context.open(pathname, flags, mode);
}

auto coContext::openat(const std::int32_t directoryFileDescriptor, const std::string_view pathname,
                       const std::int32_t flags, const std::uint32_t mode) -> AsyncWaiter {
    return context.open(directoryFileDescriptor, pathname, flags, mode);
}

auto coContext::openat2(const std::int32_t directoryFileDescriptor, const std::string_view pathname,
                        open_how *const how) -> AsyncWaiter {
    return context.open(directoryFileDescriptor, pathname, how);
}

auto coContext::read(const std::int32_t fileDescriptor, const std::span<std::byte> buffer) -> AsyncWaiter {
    return pread(fileDescriptor, buffer, -1);
}

auto coContext::readv(const std::int32_t fileDescriptor, const std::span<const iovec> buffer) -> AsyncWaiter {
    return preadv(fileDescriptor, buffer, -1);
}

auto coContext::pread(const std::int32_t fileDescriptor, const std::span<std::byte> buffer, const std::uint64_t offset)
    -> AsyncWaiter {
    return context.read(fileDescriptor, buffer, offset);
}

auto coContext::preadv(const std::int32_t fileDescriptor, const std::span<const iovec> buffer,
                       const std::uint64_t offset) -> AsyncWaiter {
    return context.read(fileDescriptor, buffer, offset);
}

#include "coContext/coContext.hpp"

#include "coroutine/Context.hpp"

namespace {
    thread_local coContext::Context context;
}

auto coContext::spawn(Task &&task) -> void { context.spawn(std::move(task)); }

auto coContext::run() -> void { context.run(); }

auto coContext::stop() -> AsyncWaiter { return AsyncWaiter{context.stop()}; }

auto coContext::syncCancel(const std::uint64_t taskHash, const __kernel_timespec timeout) -> std::int32_t {
    return context.cancel(taskHash, 0, timeout);
}

auto coContext::syncCancel(const std::int32_t fileDescriptor, const bool isMatchAll, const __kernel_timespec timeout)
    -> std::int32_t {
    return context.cancel(fileDescriptor, isMatchAll ? IORING_ASYNC_CANCEL_ALL : 0, timeout);
}

auto coContext::syncCancelAny(const __kernel_timespec timeout) -> std::int32_t {
    return context.cancel(std::uint64_t{}, IORING_ASYNC_CANCEL_ANY, timeout);
}

auto coContext::cancel(const std::uint64_t taskHash) -> AsyncWaiter { return AsyncWaiter{context.cancel(taskHash, 0)}; }

auto coContext::cancel(const std::int32_t fileDescriptor, const bool isMatchAll) -> AsyncWaiter {
    return AsyncWaiter{context.cancel(fileDescriptor, isMatchAll ? IORING_ASYNC_CANCEL_ALL : 0)};
}

auto coContext::cancelAny() -> AsyncWaiter {
    return AsyncWaiter{context.cancel(std::uint64_t{}, IORING_ASYNC_CANCEL_ANY)};
}

auto coContext::timeout(__kernel_timespec &timeout, ClockSource clockSource) -> AsyncWaiter {
    std::uint32_t flags{};
    switch (clockSource) {
        case ClockSource::monotonic:
            break;
        case ClockSource::absolute:
            flags = IORING_TIMEOUT_ABS;
            break;
        case ClockSource::boot:
            flags = IORING_TIMEOUT_BOOTTIME;
            break;
        case ClockSource::real:
            flags = IORING_TIMEOUT_REALTIME;
    }

    return AsyncWaiter{context.timeout(timeout, 0, flags)};
}

auto coContext::updateTimeout(__kernel_timespec &timeout, const std::uint64_t taskHash, const ClockSource clockSource)
    -> AsyncWaiter {
    std::uint32_t flags{};
    switch (clockSource) {
        case ClockSource::monotonic:
            break;
        case ClockSource::absolute:
            flags = IORING_TIMEOUT_ABS;
            break;
        case ClockSource::boot:
            flags = IORING_TIMEOUT_BOOTTIME;
            break;
        case ClockSource::real:
            flags = IORING_TIMEOUT_REALTIME;
    }

    return AsyncWaiter{context.updateTimeout(timeout, taskHash, flags)};
}

auto coContext::close(const std::int32_t fileDescriptor) -> AsyncWaiter {
    return AsyncWaiter{context.close(fileDescriptor)};
}

auto coContext::socket(const std::int32_t domain, const std::int32_t type, const std::int32_t protocol) -> AsyncWaiter {
    return AsyncWaiter{context.socket(domain, type, protocol)};
}

auto coContext::bind(const std::int32_t socketFileDescriptor, sockaddr *const address,
                     const std::uint32_t addressLength) -> AsyncWaiter {
    return AsyncWaiter{context.bind(socketFileDescriptor, address, addressLength)};
}

auto coContext::listen(const std::int32_t socketFileDescriptor, const std::int32_t backlog) -> AsyncWaiter {
    return AsyncWaiter{context.listen(socketFileDescriptor, backlog)};
}

auto coContext::accept(const std::int32_t socketFileDescriptor, sockaddr *const address,
                       std::uint32_t *const addressLength, const std::int32_t flags) -> AsyncWaiter {
    return AsyncWaiter{context.accept(socketFileDescriptor, address, addressLength, flags)};
}

auto coContext::connect(const std::int32_t socketFileDescriptor, const sockaddr *const address,
                        const std::uint32_t addressLength) -> AsyncWaiter {
    return AsyncWaiter{context.connect(socketFileDescriptor, address, addressLength)};
}

auto coContext::shutdown(const std::int32_t socketFileDescriptor, const std::int32_t how) -> AsyncWaiter {
    return AsyncWaiter{context.shutdown(socketFileDescriptor, how)};
}

auto coContext::receive(const std::int32_t socketFileDescriptor, const std::span<std::byte> buffer,
                        const std::int32_t flags) -> AsyncWaiter {
    return AsyncWaiter{context.receive(socketFileDescriptor, buffer, flags)};
}

auto coContext::receive(const std::int32_t socketFileDescriptor, msghdr &message, const std::uint32_t flags)
    -> AsyncWaiter {
    return AsyncWaiter{context.receive(socketFileDescriptor, message, flags)};
}

auto coContext::send(const std::int32_t socketFileDescriptor, const std::span<const std::byte> buffer,
                     const std::int32_t flags) -> AsyncWaiter {
    return AsyncWaiter{context.send(socketFileDescriptor, buffer, flags)};
}

auto coContext::send(const std::int32_t socketFileDescriptor, const std::span<const std::byte> buffer,
                     const std::int32_t flags, const sockaddr *const address, const std::uint32_t addressLength)
    -> AsyncWaiter {
    return AsyncWaiter{context.send(socketFileDescriptor, buffer, flags, address, addressLength)};
}

auto coContext::send(const std::int32_t socketFileDescriptor, const msghdr &message, const std::uint32_t flags)
    -> AsyncWaiter {
    return AsyncWaiter{context.send(socketFileDescriptor, message, flags)};
}

auto coContext::open(const std::string_view pathname, const std::int32_t flags, const std::uint32_t mode)
    -> AsyncWaiter {
    return AsyncWaiter{context.open(pathname, flags, mode)};
}

auto coContext::open(const std::int32_t directoryFileDescriptor, const std::string_view pathname,
                     const std::int32_t flags, const std::uint32_t mode) -> AsyncWaiter {
    return AsyncWaiter{context.open(directoryFileDescriptor, pathname, flags, mode)};
}

auto coContext::open(const std::int32_t directoryFileDescriptor, const std::string_view pathname, open_how &how)
    -> AsyncWaiter {
    return AsyncWaiter{context.open(directoryFileDescriptor, pathname, how)};
}

auto coContext::read(const std::int32_t fileDescriptor, const std::span<std::byte> buffer, const std::uint64_t offset)
    -> AsyncWaiter {
    return AsyncWaiter{context.read(fileDescriptor, buffer, offset)};
}

auto coContext::read(const std::int32_t fileDescriptor, const std::span<const iovec> buffer, const std::uint64_t offset)
    -> AsyncWaiter {
    return AsyncWaiter{context.read(fileDescriptor, buffer, offset)};
}

auto coContext::read(const std::int32_t fileDescriptor, const std::span<const iovec> buffer, const std::uint64_t offset,
                     const std::int32_t flags) -> AsyncWaiter {
    return AsyncWaiter{context.read(fileDescriptor, buffer, offset, flags)};
}

auto coContext::write(const std::int32_t fileDescriptor, const std::span<const std::byte> buffer,
                      const std::uint64_t offset) -> AsyncWaiter {
    return AsyncWaiter{context.write(fileDescriptor, buffer, offset)};
}

auto coContext::write(const std::int32_t fileDescriptor, const std::span<const iovec> buffer,
                      const std::uint64_t offset) -> AsyncWaiter {
    return AsyncWaiter{context.write(fileDescriptor, buffer, offset)};
}

auto coContext::write(const std::int32_t fileDescriptor, const std::span<const iovec> buffer,
                      const std::uint64_t offset, const std::int32_t flags) -> AsyncWaiter {
    return AsyncWaiter{context.write(fileDescriptor, buffer, offset, flags)};
}

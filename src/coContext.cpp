#include "coContext/coContext.hpp"

#include "coroutine/Context.hpp"

namespace {
    thread_local coContext::Context context;

    [[nodiscard]] constexpr auto setClockSource(const coContext::ClockSource clockSource) noexcept -> std::uint32_t {
        std::uint32_t flags{};
        switch (clockSource) {
            case coContext::ClockSource::monotonic:
                break;
            case coContext::ClockSource::absolute:
                flags = IORING_TIMEOUT_ABS;
                break;
            case coContext::ClockSource::boot:
                flags = IORING_TIMEOUT_BOOTTIME;
                break;
            case coContext::ClockSource::real:
                flags = IORING_TIMEOUT_REALTIME;
                break;
        }

        return flags;
    }
}    // namespace

auto coContext::spawn(GenericTask &&task) -> void { context.spawn(std::move(task)); }

auto coContext::spawn(Task<> &&task) -> void { spawn(GenericTask{std::move(task.getCoroutine())}); }

auto coContext::run() -> void { context.run(); }

auto coContext::stop() -> AsyncWaiter { return context.stop(); }

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

auto coContext::cancel(const std::uint64_t taskHash) -> AsyncWaiter { return context.cancel(taskHash, 0); }

auto coContext::cancel(const std::int32_t fileDescriptor, const bool isMatchAll) -> AsyncWaiter {
    return context.cancel(fileDescriptor, isMatchAll ? IORING_ASYNC_CANCEL_ALL : 0);
}

auto coContext::cancelAny() -> AsyncWaiter { return context.cancel(std::uint64_t{}, IORING_ASYNC_CANCEL_ANY); }

auto coContext::timeout(__kernel_timespec &timeout, const ClockSource clockSource) -> AsyncWaiter {
    return context.timeout(timeout, 0, setClockSource(clockSource));
}

auto coContext::updateTimeout(__kernel_timespec &timeout, const std::uint64_t taskHash, const ClockSource clockSource)
    -> AsyncWaiter {
    return context.updateTimeout(timeout, taskHash, setClockSource(clockSource));
}

auto coContext::removeTimeout(const std::uint64_t taskHash) -> AsyncWaiter { return context.removeTimeout(taskHash); }

auto coContext::close(const std::int32_t fileDescriptor) -> AsyncWaiter { return context.close(fileDescriptor); }

auto coContext::socket(const std::int32_t domain, const std::int32_t type, const std::int32_t protocol) -> AsyncWaiter {
    return context.socket(domain, type, protocol);
}

auto coContext::bind(const std::int32_t socketFileDescriptor, sockaddr *const address,
                     const std::uint32_t addressLength) -> AsyncWaiter {
    return context.bind(socketFileDescriptor, address, addressLength);
}

auto coContext::listen(const std::int32_t socketFileDescriptor, const std::int32_t backlog) -> AsyncWaiter {
    return context.listen(socketFileDescriptor, backlog);
}

auto coContext::accept(const std::int32_t socketFileDescriptor, sockaddr *const address,
                       std::uint32_t *const addressLength, const std::int32_t flags) -> AsyncWaiter {
    return context.accept(socketFileDescriptor, address, addressLength, flags);
}

auto coContext::connect(const std::int32_t socketFileDescriptor, const sockaddr *const address,
                        const std::uint32_t addressLength) -> AsyncWaiter {
    return context.connect(socketFileDescriptor, address, addressLength);
}

auto coContext::shutdown(const std::int32_t socketFileDescriptor, const std::int32_t how) -> AsyncWaiter {
    return context.shutdown(socketFileDescriptor, how);
}

auto coContext::receive(const std::int32_t socketFileDescriptor, const std::span<std::byte> buffer,
                        const std::int32_t flags) -> AsyncWaiter {
    return context.receive(socketFileDescriptor, buffer, flags);
}

auto coContext::receive(const std::int32_t socketFileDescriptor, msghdr &message, const std::uint32_t flags)
    -> AsyncWaiter {
    return context.receive(socketFileDescriptor, message, flags);
}

auto coContext::send(const std::int32_t socketFileDescriptor, const std::span<const std::byte> buffer,
                     const std::int32_t flags) -> AsyncWaiter {
    return context.send(socketFileDescriptor, buffer, flags);
}

auto coContext::send(const std::int32_t socketFileDescriptor, const std::span<const std::byte> buffer,
                     const std::int32_t flags, const sockaddr *const address, const std::uint32_t addressLength)
    -> AsyncWaiter {
    return context.send(socketFileDescriptor, buffer, flags, address, addressLength);
}

auto coContext::send(const std::int32_t socketFileDescriptor, const msghdr &message, const std::uint32_t flags)
    -> AsyncWaiter {
    return context.send(socketFileDescriptor, message, flags);
}

auto coContext::open(const std::string_view pathname, const std::int32_t flags, const std::uint32_t mode)
    -> AsyncWaiter {
    return context.open(pathname, flags, mode);
}

auto coContext::open(const std::int32_t directoryFileDescriptor, const std::string_view pathname,
                     const std::int32_t flags, const std::uint32_t mode) -> AsyncWaiter {
    return context.open(directoryFileDescriptor, pathname, flags, mode);
}

auto coContext::open(const std::int32_t directoryFileDescriptor, const std::string_view pathname, open_how &how)
    -> AsyncWaiter {
    return context.open(directoryFileDescriptor, pathname, how);
}

auto coContext::read(const std::int32_t fileDescriptor, const std::span<std::byte> buffer, const std::uint64_t offset)
    -> AsyncWaiter {
    return context.read(fileDescriptor, buffer, offset);
}

auto coContext::read(const std::int32_t fileDescriptor, const std::span<const iovec> buffer, const std::uint64_t offset)
    -> AsyncWaiter {
    return context.read(fileDescriptor, buffer, offset);
}

auto coContext::read(const std::int32_t fileDescriptor, const std::span<const iovec> buffer, const std::uint64_t offset,
                     const std::int32_t flags) -> AsyncWaiter {
    return context.read(fileDescriptor, buffer, offset, flags);
}

auto coContext::write(const std::int32_t fileDescriptor, const std::span<const std::byte> buffer,
                      const std::uint64_t offset) -> AsyncWaiter {
    return context.write(fileDescriptor, buffer, offset);
}

auto coContext::write(const std::int32_t fileDescriptor, const std::span<const iovec> buffer,
                      const std::uint64_t offset) -> AsyncWaiter {
    return context.write(fileDescriptor, buffer, offset);
}

auto coContext::write(const std::int32_t fileDescriptor, const std::span<const iovec> buffer,
                      const std::uint64_t offset, const std::int32_t flags) -> AsyncWaiter {
    return context.write(fileDescriptor, buffer, offset, flags);
}

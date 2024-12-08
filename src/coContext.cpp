#include "coContext/coContext.hpp"

#include "context/Context.hpp"

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

auto coContext::run() -> void { context.run(); }

auto coContext::stop() noexcept -> void { return context.stop(); }

auto coContext::syncCancel(const std::uint64_t taskIdentity, const __kernel_timespec timeSpecification)
    -> std::int32_t {
    return context.syncCancel(taskIdentity, 0, timeSpecification);
}

auto coContext::syncCancel(const std::int32_t fileDescriptor, const bool isMatchAll,
                           const __kernel_timespec timeSpecification) -> std::int32_t {
    return context.syncCancel(fileDescriptor, isMatchAll ? IORING_ASYNC_CANCEL_ALL : 0, timeSpecification);
}

auto coContext::syncCancelAny(const __kernel_timespec timeSpecification) -> std::int32_t {
    return context.syncCancel(std::uint64_t{}, IORING_ASYNC_CANCEL_ANY, timeSpecification);
}

auto coContext::cancel(const std::uint64_t taskIdentity) -> AsyncWaiter {
    SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.cancel(taskIdentity, 0);

    return {context.getConstSchedulingTasks(), std::move(submissionQueueEntry)};
}

auto coContext::cancel(const std::int32_t fileDescriptor, const bool isMatchAll) -> AsyncWaiter {
    SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.cancel(fileDescriptor, isMatchAll ? IORING_ASYNC_CANCEL_ALL : 0);

    return {context.getConstSchedulingTasks(), std::move(submissionQueueEntry)};
}

auto coContext::cancelAny() -> AsyncWaiter {
    SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.cancel(std::uint64_t{}, std::int32_t{IORING_ASYNC_CANCEL_ANY});

    return {context.getConstSchedulingTasks(), std::move(submissionQueueEntry)};
}

auto coContext::sleep(__kernel_timespec &timeSpecification, const ClockSource clockSource) -> AsyncWaiter {
    SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.timeout(timeSpecification, 0, setClockSource(clockSource));

    return {context.getConstSchedulingTasks(), std::move(submissionQueueEntry)};
}

auto coContext::updateSleep(__kernel_timespec &timeSpecification, const std::uint64_t taskIdentity,
                            const ClockSource clockSource) -> AsyncWaiter {
    SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.updateTimeout(timeSpecification, taskIdentity, setClockSource(clockSource));

    return {context.getConstSchedulingTasks(), std::move(submissionQueueEntry)};
}

auto coContext::removeSleep(const std::uint64_t taskIdentity) -> AsyncWaiter {
    SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.removeTimeout(taskIdentity);

    return {context.getConstSchedulingTasks(), std::move(submissionQueueEntry)};
}

auto coContext::close(const std::int32_t fileDescriptor) -> AsyncWaiter {
    SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.close(fileDescriptor);

    return {context.getConstSchedulingTasks(), std::move(submissionQueueEntry)};
}

auto coContext::socket(const std::int32_t domain, const std::int32_t type, const std::int32_t protocol) -> AsyncWaiter {
    SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.socket(domain, type, protocol);

    return {context.getConstSchedulingTasks(), std::move(submissionQueueEntry)};
}

auto coContext::bind(const std::int32_t socketFileDescriptor, sockaddr *const address,
                     const std::uint32_t addressLength) -> AsyncWaiter {
    SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.bind(socketFileDescriptor, address, addressLength);

    return {context.getConstSchedulingTasks(), std::move(submissionQueueEntry)};
}

auto coContext::listen(const std::int32_t socketFileDescriptor, const std::int32_t backlog) -> AsyncWaiter {
    SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.listen(socketFileDescriptor, backlog);

    return {context.getConstSchedulingTasks(), std::move(submissionQueueEntry)};
}

auto coContext::accept(const std::int32_t socketFileDescriptor, sockaddr *const address,
                       std::uint32_t *const addressLength, const std::int32_t flags) -> AsyncWaiter {
    SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.accept(socketFileDescriptor, address, addressLength, flags);
    submissionQueueEntry.addIoPriority(IORING_ACCEPT_POLL_FIRST);

    return {context.getConstSchedulingTasks(), std::move(submissionQueueEntry)};
}

auto coContext::connect(const std::int32_t socketFileDescriptor, const sockaddr *const address,
                        const std::uint32_t addressLength) -> AsyncWaiter {
    SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.connect(socketFileDescriptor, address, addressLength);

    return {context.getConstSchedulingTasks(), std::move(submissionQueueEntry)};
}

auto coContext::shutdown(const std::int32_t socketFileDescriptor, const std::int32_t how) -> AsyncWaiter {
    SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.shutdown(socketFileDescriptor, how);

    return {context.getConstSchedulingTasks(), std::move(submissionQueueEntry)};
}

auto coContext::receive(const std::int32_t socketFileDescriptor, const std::span<std::byte> buffer,
                        const std::int32_t flags) -> AsyncWaiter {
    SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.receive(socketFileDescriptor, buffer, flags);
    submissionQueueEntry.addIoPriority(IORING_RECVSEND_POLL_FIRST);

    return {context.getConstSchedulingTasks(), std::move(submissionQueueEntry)};
}

auto coContext::receive(const std::int32_t socketFileDescriptor, msghdr &message, const std::uint32_t flags)
    -> AsyncWaiter {
    SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.receive(socketFileDescriptor, message, flags);
    submissionQueueEntry.addIoPriority(IORING_RECVSEND_POLL_FIRST);

    return {context.getConstSchedulingTasks(), std::move(submissionQueueEntry)};
}

auto coContext::send(const std::int32_t socketFileDescriptor, const std::span<const std::byte> buffer,
                     const std::int32_t flags) -> AsyncWaiter {
    SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.send(socketFileDescriptor, buffer, flags);
    submissionQueueEntry.addIoPriority(IORING_RECVSEND_POLL_FIRST);

    return {context.getConstSchedulingTasks(), std::move(submissionQueueEntry)};
}

auto coContext::send(const std::int32_t socketFileDescriptor, const std::span<const std::byte> buffer,
                     const std::int32_t flags, const sockaddr *const address, const std::uint32_t addressLength)
    -> AsyncWaiter {
    SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.send(socketFileDescriptor, buffer, flags, address, addressLength);

    return {context.getConstSchedulingTasks(), std::move(submissionQueueEntry)};
}

auto coContext::send(const std::int32_t socketFileDescriptor, const msghdr &message, const std::uint32_t flags)
    -> AsyncWaiter {
    SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.send(socketFileDescriptor, message, flags);
    submissionQueueEntry.addIoPriority(IORING_RECVSEND_POLL_FIRST);

    return {context.getConstSchedulingTasks(), std::move(submissionQueueEntry)};
}

auto coContext::open(const std::string_view pathname, const std::int32_t flags, const std::uint32_t mode)
    -> AsyncWaiter {
    SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.open(pathname, flags, mode);

    return {context.getConstSchedulingTasks(), std::move(submissionQueueEntry)};
}

auto coContext::open(const std::int32_t directoryFileDescriptor, const std::string_view pathname,
                     const std::int32_t flags, const std::uint32_t mode) -> AsyncWaiter {
    SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.open(directoryFileDescriptor, pathname, flags, mode);

    return {context.getConstSchedulingTasks(), std::move(submissionQueueEntry)};
}

auto coContext::open(const std::int32_t directoryFileDescriptor, const std::string_view pathname, open_how &how)
    -> AsyncWaiter {
    SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.open(directoryFileDescriptor, pathname, how);

    return {context.getConstSchedulingTasks(), std::move(submissionQueueEntry)};
}

auto coContext::read(const std::int32_t fileDescriptor, const std::span<std::byte> buffer, const std::uint64_t offset)
    -> AsyncWaiter {
    SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.read(fileDescriptor, buffer, offset);

    return {context.getConstSchedulingTasks(), std::move(submissionQueueEntry)};
}

auto coContext::read(const std::int32_t fileDescriptor, const std::span<const iovec> buffer, const std::uint64_t offset)
    -> AsyncWaiter {
    SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.read(fileDescriptor, buffer, offset);

    return {context.getConstSchedulingTasks(), std::move(submissionQueueEntry)};
}

auto coContext::read(const std::int32_t fileDescriptor, const std::span<const iovec> buffer, const std::uint64_t offset,
                     const std::int32_t flags) -> AsyncWaiter {
    SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.read(fileDescriptor, buffer, offset, flags);

    return {context.getConstSchedulingTasks(), std::move(submissionQueueEntry)};
}

auto coContext::write(const std::int32_t fileDescriptor, const std::span<const std::byte> buffer,
                      const std::uint64_t offset) -> AsyncWaiter {
    SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.write(fileDescriptor, buffer, offset);

    return {context.getConstSchedulingTasks(), std::move(submissionQueueEntry)};
}

auto coContext::write(const std::int32_t fileDescriptor, const std::span<const iovec> buffer,
                      const std::uint64_t offset) -> AsyncWaiter {
    SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.write(fileDescriptor, buffer, offset);

    return {context.getConstSchedulingTasks(), std::move(submissionQueueEntry)};
}

auto coContext::write(const std::int32_t fileDescriptor, const std::span<const iovec> buffer,
                      const std::uint64_t offset, const std::int32_t flags) -> AsyncWaiter {
    SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.write(fileDescriptor, buffer, offset, flags);

    return {context.getConstSchedulingTasks(), std::move(submissionQueueEntry)};
}

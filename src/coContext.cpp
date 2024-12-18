#include "coContext/coContext.hpp"

#include "context/Context.hpp"

namespace {
    thread_local coContext::Context context;

    [[nodiscard]] constexpr auto setClockSource(const coContext::ClockSource clockSource) noexcept {
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

auto coContext::spawn(Coroutine &&coroutine) -> void { context.spawn(std::move(coroutine)); }

auto coContext::run() -> void { context.run(); }

auto coContext::stop() noexcept -> void { return context.stop(); }

auto coContext::syncCancel(const std::uint64_t taskIdentity, const std::chrono::seconds seconds,
                           const std::chrono::nanoseconds nanoseconds) -> std::int32_t {
    return context.syncCancel(taskIdentity, 0, __kernel_timespec{seconds.count(), nanoseconds.count()});
}

auto coContext::syncCancel(const std::int32_t fileDescriptor, const bool isMatchAll, const std::chrono::seconds seconds,
                           const std::chrono::nanoseconds nanoseconds) -> std::int32_t {
    return context.syncCancel(fileDescriptor, isMatchAll ? IORING_ASYNC_CANCEL_ALL : 0,
                              __kernel_timespec{seconds.count(), nanoseconds.count()});
}

auto coContext::syncCancelAny(const std::chrono::seconds seconds, const std::chrono::nanoseconds nanoseconds)
    -> std::int32_t {
    return context.syncCancel(std::uint64_t{}, IORING_ASYNC_CANCEL_ANY,
                              __kernel_timespec{seconds.count(), nanoseconds.count()});
}

auto coContext::cancel(const std::uint64_t taskIdentity) -> AsyncWaiter {
    const SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.cancel(taskIdentity, 0);

    return AsyncWaiter{submissionQueueEntry};
}

auto coContext::cancel(const std::int32_t fileDescriptor, const bool isMatchAll) -> AsyncWaiter {
    const SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.cancel(fileDescriptor, isMatchAll ? IORING_ASYNC_CANCEL_ALL : 0);

    return AsyncWaiter{submissionQueueEntry};
}

auto coContext::cancelAny() -> AsyncWaiter {
    const SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.cancel(std::uint64_t{}, std::int32_t{IORING_ASYNC_CANCEL_ANY});

    return AsyncWaiter{submissionQueueEntry};
}

auto coContext::sleep(const std::chrono::seconds seconds, const std::chrono::nanoseconds nanoseconds,
                      const ClockSource clockSource) -> AsyncWaiter {
    const SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    AsyncWaiter asyncWaiter{submissionQueueEntry};

    asyncWaiter.setFirstTimeSpecification(seconds, nanoseconds);
    submissionQueueEntry.timeout(asyncWaiter.getFirstTimeSpecification(), 0, setClockSource(clockSource));

    return asyncWaiter;
}

auto coContext::updateSleep(const std::uint64_t taskIdentity, const std::chrono::seconds seconds,
                            const std::chrono::nanoseconds nanoseconds, const ClockSource clockSource) -> AsyncWaiter {
    const SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    AsyncWaiter asyncWaiter{submissionQueueEntry};

    asyncWaiter.setFirstTimeSpecification(seconds, nanoseconds);
    submissionQueueEntry.updateTimeout(asyncWaiter.getFirstTimeSpecification(), taskIdentity,
                                       setClockSource(clockSource));

    return asyncWaiter;
}

auto coContext::timeout(AsyncWaiter &&asyncWaiter, const std::chrono::seconds seconds,
                        const std::chrono::nanoseconds nanoseconds, const ClockSource clockSource) -> AsyncWaiter {
    asyncWaiter.getSubmissionQueueEntry().addFlags(IOSQE_IO_LINK);
    asyncWaiter.setSecondTimeSpecification(seconds, nanoseconds);

    context.getSubmissionQueueEntry().linkTimeout(asyncWaiter.getSecondTimeSpecification(),
                                                  setClockSource(clockSource));

    return asyncWaiter;
}

auto coContext::close(const std::int32_t fileDescriptor) -> AsyncWaiter {
    const SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.close(fileDescriptor);

    return AsyncWaiter{submissionQueueEntry};
}

auto coContext::socket(const std::int32_t domain, const std::int32_t type, const std::int32_t protocol) -> AsyncWaiter {
    const SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.socket(domain, type, protocol, 0);

    return AsyncWaiter{submissionQueueEntry};
}

auto coContext::bind(const std::int32_t socketFileDescriptor, sockaddr *const address, const socklen_t addressLength)
    -> AsyncWaiter {
    const SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.bind(socketFileDescriptor, address, addressLength);

    return AsyncWaiter{submissionQueueEntry};
}

auto coContext::listen(const std::int32_t socketFileDescriptor, const std::int32_t backlog) -> AsyncWaiter {
    const SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.listen(socketFileDescriptor, backlog);

    return AsyncWaiter{submissionQueueEntry};
}

auto coContext::accept(const std::int32_t socketFileDescriptor, sockaddr *const address, socklen_t *const addressLength,
                       const std::int32_t flags) -> AsyncWaiter {
    const SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.accept(socketFileDescriptor, address, addressLength, flags);
    submissionQueueEntry.addIoPriority(IORING_ACCEPT_POLL_FIRST);

    return AsyncWaiter{submissionQueueEntry};
}

auto coContext::connect(const std::int32_t socketFileDescriptor, const sockaddr *const address,
                        const socklen_t addressLength) -> AsyncWaiter {
    const SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.connect(socketFileDescriptor, address, addressLength);

    return AsyncWaiter{submissionQueueEntry};
}

auto coContext::shutdown(const std::int32_t socketFileDescriptor, const std::int32_t how) -> AsyncWaiter {
    const SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.shutdown(socketFileDescriptor, how);

    return AsyncWaiter{submissionQueueEntry};
}

auto coContext::receive(const std::int32_t socketFileDescriptor, const std::span<std::byte> buffer,
                        const std::int32_t flags) -> AsyncWaiter {
    const SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.receive(socketFileDescriptor, buffer, flags);
    submissionQueueEntry.addIoPriority(IORING_RECVSEND_POLL_FIRST);

    return AsyncWaiter{submissionQueueEntry};
}

auto coContext::receive(const std::int32_t socketFileDescriptor, msghdr &message, const std::uint32_t flags)
    -> AsyncWaiter {
    const SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.receive(socketFileDescriptor, message, flags);
    submissionQueueEntry.addIoPriority(IORING_RECVSEND_POLL_FIRST);

    return AsyncWaiter{submissionQueueEntry};
}

auto coContext::send(const std::int32_t socketFileDescriptor, const std::span<const std::byte> buffer,
                     const std::int32_t flags) -> AsyncWaiter {
    const SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.send(socketFileDescriptor, buffer, flags);
    submissionQueueEntry.addIoPriority(IORING_RECVSEND_POLL_FIRST);

    return AsyncWaiter{submissionQueueEntry};
}

auto coContext::send(const std::int32_t socketFileDescriptor, const std::span<const std::byte> buffer,
                     const std::int32_t flags, const sockaddr *const address, const socklen_t addressLength)
    -> AsyncWaiter {
    const SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.send(socketFileDescriptor, buffer, flags, address, addressLength);

    return AsyncWaiter{submissionQueueEntry};
}

auto coContext::send(const std::int32_t socketFileDescriptor, const msghdr &message, const std::uint32_t flags)
    -> AsyncWaiter {
    const SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.send(socketFileDescriptor, message, flags);
    submissionQueueEntry.addIoPriority(IORING_RECVSEND_POLL_FIRST);

    return AsyncWaiter{submissionQueueEntry};
}

auto coContext::open(const std::string_view pathname, const std::int32_t flags, const mode_t mode) -> AsyncWaiter {
    const SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.open(pathname, flags, mode);

    return AsyncWaiter{submissionQueueEntry};
}

auto coContext::open(const std::int32_t directoryFileDescriptor, const std::string_view pathname,
                     const std::int32_t flags, const mode_t mode) -> AsyncWaiter {
    const SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.open(directoryFileDescriptor, pathname, flags, mode);

    return AsyncWaiter{submissionQueueEntry};
}

auto coContext::open(const std::int32_t directoryFileDescriptor, const std::string_view pathname, open_how &how)
    -> AsyncWaiter {
    const SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.open(directoryFileDescriptor, pathname, how);

    return AsyncWaiter{submissionQueueEntry};
}

auto coContext::read(const std::int32_t fileDescriptor, const std::span<std::byte> buffer, const std::uint64_t offset)
    -> AsyncWaiter {
    const SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.read(fileDescriptor, buffer, offset);

    return AsyncWaiter{submissionQueueEntry};
}

auto coContext::read(const std::int32_t fileDescriptor, const std::span<const iovec> buffer, const std::uint64_t offset)
    -> AsyncWaiter {
    const SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.read(fileDescriptor, buffer, offset);

    return AsyncWaiter{submissionQueueEntry};
}

auto coContext::read(const std::int32_t fileDescriptor, const std::span<const iovec> buffer, const std::uint64_t offset,
                     const std::int32_t flags) -> AsyncWaiter {
    const SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.read(fileDescriptor, buffer, offset, flags);

    return AsyncWaiter{submissionQueueEntry};
}

auto coContext::write(const std::int32_t fileDescriptor, const std::span<const std::byte> buffer,
                      const std::uint64_t offset) -> AsyncWaiter {
    const SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.write(fileDescriptor, buffer, offset);

    return AsyncWaiter{submissionQueueEntry};
}

auto coContext::write(const std::int32_t fileDescriptor, const std::span<const iovec> buffer,
                      const std::uint64_t offset) -> AsyncWaiter {
    const SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.write(fileDescriptor, buffer, offset);

    return AsyncWaiter{submissionQueueEntry};
}

auto coContext::write(const std::int32_t fileDescriptor, const std::span<const iovec> buffer,
                      const std::uint64_t offset, const std::int32_t flags) -> AsyncWaiter {
    const SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.write(fileDescriptor, buffer, offset, flags);

    return AsyncWaiter{submissionQueueEntry};
}

auto coContext::fileSync(const std::int32_t fileDescriptor, const bool isSyncMetadata, const std::uint64_t offset,
                         const std::uint32_t length) -> AsyncWaiter {
    const SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.fileSync(fileDescriptor, isSyncMetadata ? 0 : IORING_FSYNC_DATASYNC, offset, length);

    return AsyncWaiter{submissionQueueEntry};
}

auto coContext::link(const std::string_view oldPath, const std::string_view newPath) -> AsyncWaiter {
    const SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.link(oldPath, newPath, 0);

    return AsyncWaiter{submissionQueueEntry};
}

auto coContext::link(const std::int32_t oldDirectoryFileDescriptor, const std::string_view oldPath,
                     const std::int32_t newDirectoryFileDescriptor, const std::string_view newPath,
                     const std::int32_t flags) -> AsyncWaiter {
    const SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.link(oldDirectoryFileDescriptor, oldPath, newDirectoryFileDescriptor, newPath, flags);

    return AsyncWaiter{submissionQueueEntry};
}

auto coContext::symbolicLink(const std::string_view target, const std::string_view linkPath) -> AsyncWaiter {
    const SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.symbolicLink(target, linkPath);

    return AsyncWaiter{submissionQueueEntry};
}

auto coContext::symbolicLink(const std::string_view target, const std::int32_t newDirectoryFileDescriptor,
                             const std::string_view linkPath) -> AsyncWaiter {
    const SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.symbolicLink(target, newDirectoryFileDescriptor, linkPath);

    return AsyncWaiter{submissionQueueEntry};
}

auto coContext::unlink(const std::string_view path) -> AsyncWaiter {
    const SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.unlink(path, 0);

    return AsyncWaiter{submissionQueueEntry};
}

auto coContext::unlink(const std::int32_t directoryFileDescriptor, const std::string_view path,
                       const std::int32_t flags) -> AsyncWaiter {
    const SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.unlink(directoryFileDescriptor, path, flags);

    return AsyncWaiter{submissionQueueEntry};
}

auto coContext::makeDirectory(const std::string_view path, const mode_t mode) -> AsyncWaiter {
    const SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.makeDirectory(path, mode);

    return AsyncWaiter{submissionQueueEntry};
}

auto coContext::makeDirectory(const std::int32_t directoryFileDescriptor, const std::string_view path,
                              const mode_t mode) -> AsyncWaiter {
    const SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.makeDirectory(directoryFileDescriptor, path, mode);

    return AsyncWaiter{submissionQueueEntry};
}

auto coContext::rename(const std::string_view oldPath, const std::string_view newPath) -> AsyncWaiter {
    const SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.rename(oldPath, newPath);

    return AsyncWaiter{submissionQueueEntry};
}

auto coContext::rename(const std::int32_t oldDirectoryFileDescriptor, const std::string_view oldPath,
                       const std::int32_t newDirectoryFileDescriptor, const std::string_view newPath,
                       const std::uint32_t flags) -> AsyncWaiter {
    const SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.rename(oldDirectoryFileDescriptor, oldPath, newDirectoryFileDescriptor, newPath, flags);

    return AsyncWaiter{submissionQueueEntry};
}

auto coContext::truncate(const std::int32_t fileDescriptor, const loff_t length) -> AsyncWaiter {
    const SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.truncate(fileDescriptor, length);

    return AsyncWaiter{submissionQueueEntry};
}

auto coContext::allocateFile(const std::int32_t fileDescriptor, const std::int32_t mode, const std::uint64_t offset,
                             const std::uint64_t length) -> AsyncWaiter {
    const SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.allocateFile(fileDescriptor, mode, offset, length);

    return AsyncWaiter{submissionQueueEntry};
}

auto coContext::getFileStatus(const std::int32_t directoryFileDescriptor, const std::string_view path,
                              const std::int32_t flags, const std::uint32_t mask, struct statx &buffer) -> AsyncWaiter {
    const SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.getFileStatus(directoryFileDescriptor, path, flags, mask, buffer);

    return AsyncWaiter{submissionQueueEntry};
}

auto coContext::getExtendedAttribute(const std::string_view path, const std::string_view name,
                                     const std::span<char> value) -> AsyncWaiter {
    const SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.getExtendedAttribute(path, name, value);

    return AsyncWaiter{submissionQueueEntry};
}

auto coContext::getExtendedAttribute(const std::int32_t fileDescriptor, const std::string_view name,
                                     const std::span<char> value) -> AsyncWaiter {
    const SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.getExtendedAttribute(fileDescriptor, name, value);

    return AsyncWaiter{submissionQueueEntry};
}

auto coContext::setExtendedAttribute(const std::string_view path, const std::string_view name,
                                     const std::span<char> value, const std::int32_t flags) -> AsyncWaiter {
    const SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.setExtendedAttribute(path, name, value, flags);

    return AsyncWaiter{submissionQueueEntry};
}

auto coContext::setExtendedAttribute(const std::int32_t fileDescriptor, const std::string_view name,
                                     const std::span<char> value, const std::int32_t flags) -> AsyncWaiter {
    const SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.setExtendedAttribute(fileDescriptor, name, value, flags);

    return AsyncWaiter{submissionQueueEntry};
}

auto coContext::memoryAdvise(const std::span<std::byte> buffer, const std::int32_t advice) -> AsyncWaiter {
    const SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.memoryAdvise(buffer, advice);

    return AsyncWaiter{submissionQueueEntry};
}

auto coContext::fileAdvise(const std::int32_t fileDescriptor, const std::uint64_t offset, const off_t length,
                           const std::int32_t advice) -> AsyncWaiter {
    const SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.fileAdvise(fileDescriptor, offset, length, advice);

    return AsyncWaiter{submissionQueueEntry};
}

auto coContext::splice(const std::int32_t inFileDescriptor, const std::int64_t inFileDescriptorOffset,
                       const std::int32_t outFileDescriptor, const std::int64_t outFileDescriptorOffset,
                       const std::uint32_t numberOfBytes, const std::uint32_t flags) -> AsyncWaiter {
    const SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.splice(inFileDescriptor, inFileDescriptorOffset, outFileDescriptor, outFileDescriptorOffset,
                                numberOfBytes, flags);

    return AsyncWaiter{submissionQueueEntry};
}


#include "coContext/coContext.hpp"

#include "context/Context.hpp"

namespace {
    thread_local coContext::internal::Context context;

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

auto coContext::internal::spawn(Coroutine coroutine) -> void { context.spawn(std::move(coroutine)); }

auto coContext::run() -> void { context.run(); }

auto coContext::stop() noexcept -> void { context.stop(); }

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

auto coContext::direct() noexcept -> internal::Marker { return internal::Marker{IOSQE_FIXED_FILE}; }

auto coContext::timeout(const std::chrono::seconds seconds, const std::chrono::nanoseconds nanoseconds,
                        const ClockSource clockSource) -> internal::Marker {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    internal::AsyncWaiter asyncWaiter{submissionQueueEntry};

    asyncWaiter.setTimeSpecification(std::make_unique<__kernel_timespec>(seconds.count(), nanoseconds.count()));
    submissionQueueEntry.linkTimeout(*asyncWaiter.getTimeSpecification(), setClockSource(clockSource));

    spawn([](internal::AsyncWaiter timeoutAsyncWaiter) -> Task<> { co_await timeoutAsyncWaiter; },
          std::move(asyncWaiter));

    return internal::Marker{IOSQE_IO_LINK};
}

auto coContext::cancel(const std::uint64_t taskIdentity) -> internal::AsyncWaiter {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.cancel(taskIdentity, 0);

    return internal::AsyncWaiter{submissionQueueEntry};
}

auto coContext::cancel(const std::int32_t fileDescriptor, const bool isMatchAll) -> internal::AsyncWaiter {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.cancel(fileDescriptor, isMatchAll ? IORING_ASYNC_CANCEL_ALL : 0);

    return internal::AsyncWaiter{submissionQueueEntry};
}

auto coContext::cancelAny() -> internal::AsyncWaiter {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.cancel(std::uint64_t{}, std::int32_t{IORING_ASYNC_CANCEL_ANY});

    return internal::AsyncWaiter{submissionQueueEntry};
}

[[nodiscard]] constexpr auto sleep(const std::chrono::seconds seconds, const std::chrono::nanoseconds nanoseconds,
                                   const std::uint32_t flags) {
    const coContext::internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    coContext::internal::AsyncWaiter asyncWaiter{submissionQueueEntry};

    asyncWaiter.setTimeSpecification(std::make_unique<__kernel_timespec>(seconds.count(), nanoseconds.count()));
    submissionQueueEntry.timeout(*asyncWaiter.getTimeSpecification(), 0, flags);

    return asyncWaiter;
}

auto coContext::sleep(const std::chrono::seconds seconds, const std::chrono::nanoseconds nanoseconds,
                      const ClockSource clockSource) -> internal::AsyncWaiter {
    return ::sleep(seconds, nanoseconds, setClockSource(clockSource));
}

auto coContext::updateSleep(const std::uint64_t taskIdentity, const std::chrono::seconds seconds,
                            const std::chrono::nanoseconds nanoseconds, const ClockSource clockSource)
    -> internal::AsyncWaiter {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    internal::AsyncWaiter asyncWaiter{submissionQueueEntry};

    asyncWaiter.setTimeSpecification(std::make_unique<__kernel_timespec>(seconds.count(), nanoseconds.count()));
    submissionQueueEntry.updateTimeout(taskIdentity, *asyncWaiter.getTimeSpecification(), setClockSource(clockSource));

    return asyncWaiter;
}

auto coContext::multipleSleep(std::move_only_function<auto(std::int32_t)->void> action,
                              const std::chrono::seconds seconds, const std::chrono::nanoseconds nanoseconds,
                              const ClockSource clockSource) -> Task<> {
    internal::AsyncWaiter asyncWaiter{
        ::sleep(seconds, nanoseconds, setClockSource(clockSource) | IORING_TIMEOUT_MULTISHOT)};

    std::uint32_t asyncWaitResumeFlags{IORING_CQE_F_MORE};
    while (asyncWaitResumeFlags & IORING_CQE_F_MORE) {
        action(co_await asyncWaiter);
        asyncWaitResumeFlags = asyncWaiter.getAsyncWaitResumeFlags();
    }
}

auto coContext::poll(const std::int32_t fileDescriptor, const std::uint32_t mask) -> internal::AsyncWaiter {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.poll(fileDescriptor, mask);

    return internal::AsyncWaiter{submissionQueueEntry};
}

auto coContext::updatePoll(const std::uint64_t taskIdentity, const std::uint32_t mask) -> internal::AsyncWaiter {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.updatePoll(taskIdentity, 0, mask, IORING_POLL_UPDATE_EVENTS);

    return internal::AsyncWaiter{submissionQueueEntry};
}

auto coContext::multiplePoll(std::move_only_function<auto(std::int32_t)->void> action,
                             const std::int32_t fileDescriptor, const std::uint32_t mask) -> Task<> {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.multiplePoll(fileDescriptor, mask);

    internal::AsyncWaiter asyncWaiter{submissionQueueEntry};

    std::uint32_t asyncWaitResumeFlags{IORING_CQE_F_MORE};
    while (asyncWaitResumeFlags & IORING_CQE_F_MORE) {
        action(co_await asyncWaiter);
        asyncWaitResumeFlags = asyncWaiter.getAsyncWaitResumeFlags();
    }
}

auto coContext::installDirectFileDescriptor(const std::int32_t directFileDescriptor, const bool isSetCloseOnExec)
    -> internal::AsyncWaiter {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.installDirectFileDescriptor(directFileDescriptor,
                                                     isSetCloseOnExec ? 0 : IORING_FIXED_FD_NO_CLOEXEC);

    return internal::AsyncWaiter{submissionQueueEntry};
}

auto coContext::close(const std::int32_t fileDescriptor) -> internal::AsyncWaiter {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.close(fileDescriptor);

    return internal::AsyncWaiter{submissionQueueEntry};
}

auto coContext::closeDirect(const std::int32_t directFileDescriptor) -> internal::AsyncWaiter {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.closeDirect(directFileDescriptor);

    return internal::AsyncWaiter{submissionQueueEntry};
}

auto coContext::socket(const std::int32_t domain, const std::int32_t type, const std::int32_t protocol)
    -> internal::AsyncWaiter {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.socket(domain, type, protocol, 0);

    return internal::AsyncWaiter{submissionQueueEntry};
}

auto coContext::directSocket(const std::int32_t domain, const std::int32_t type, const std::int32_t protocol)
    -> internal::AsyncWaiter {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.directSocket(domain, type, protocol, 0);

    return internal::AsyncWaiter{submissionQueueEntry};
}

auto coContext::getSocketOption(const std::int32_t socketFileDescriptor, const std::int32_t level,
                                const std::int32_t optionName, void *const optionValue, const std::int32_t optionLength)
    -> internal::AsyncWaiter {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.socketCommand(SOCKET_URING_OP_GETSOCKOPT, socketFileDescriptor, level, optionName, optionValue,
                                       optionLength);

    return internal::AsyncWaiter{submissionQueueEntry};
}

auto coContext::setSocketOption(const std::int32_t socketFileDescriptor, const std::int32_t level,
                                const std::int32_t optionName, void *const optionValue, const std::int32_t optionLength)
    -> internal::AsyncWaiter {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.socketCommand(SOCKET_URING_OP_SETSOCKOPT, socketFileDescriptor, level, optionName, optionValue,
                                       optionLength);

    return internal::AsyncWaiter{submissionQueueEntry};
}

auto coContext::getSocketReceiveBufferUnreadDataSize(const std::int32_t socketFileDescriptor) -> internal::AsyncWaiter {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.socketCommand(SOCKET_URING_OP_SIOCINQ, socketFileDescriptor, 0, 0, nullptr, 0);

    return internal::AsyncWaiter{submissionQueueEntry};
}

auto coContext::getSocketSendBufferUnsentDataSize(const std::int32_t socketFileDescriptor) -> internal::AsyncWaiter {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.socketCommand(SOCKET_URING_OP_SIOCOUTQ, socketFileDescriptor, 0, 0, nullptr, 0);

    return internal::AsyncWaiter{submissionQueueEntry};
}

auto coContext::discardData(const std::int32_t fileDescriptor, const std::uint64_t offset, const std::uint64_t length)
    -> internal::AsyncWaiter {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.discardCommand(fileDescriptor, offset, length);

    return internal::AsyncWaiter{submissionQueueEntry};
}

auto coContext::bind(const std::int32_t socketFileDescriptor, sockaddr &address, const socklen_t addressLength)
    -> internal::AsyncWaiter {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.bind(socketFileDescriptor, address, addressLength);

    return internal::AsyncWaiter{submissionQueueEntry};
}

auto coContext::listen(const std::int32_t socketFileDescriptor, const std::int32_t backlog) -> internal::AsyncWaiter {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.listen(socketFileDescriptor, backlog);

    return internal::AsyncWaiter{submissionQueueEntry};
}

auto coContext::accept(const std::int32_t socketFileDescriptor, sockaddr *const address, socklen_t *const addressLength,
                       const std::int32_t flags) -> internal::AsyncWaiter {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.accept(socketFileDescriptor, address, addressLength, flags);
    submissionQueueEntry.addIoPriority(IORING_ACCEPT_POLL_FIRST);

    return internal::AsyncWaiter{submissionQueueEntry};
}

auto coContext::acceptDirect(const std::int32_t socketFileDescriptor, sockaddr *const address,
                             socklen_t *const addressLength, const std::int32_t flags) -> internal::AsyncWaiter {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.acceptDirect(socketFileDescriptor, address, addressLength, flags, IORING_FILE_INDEX_ALLOC);
    submissionQueueEntry.addIoPriority(IORING_ACCEPT_POLL_FIRST);

    return internal::AsyncWaiter{submissionQueueEntry};
}

auto coContext::multipleAccept(std::move_only_function<auto(std::int32_t)->void> action,
                               const std::int32_t socketFileDescriptor, sockaddr *const address,
                               socklen_t *const addressLength, const std::int32_t flags) -> Task<> {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.multipleAccept(socketFileDescriptor, address, addressLength, flags);
    submissionQueueEntry.addIoPriority(IORING_ACCEPT_POLL_FIRST);

    internal::AsyncWaiter asyncWaiter{submissionQueueEntry};

    std::uint32_t asyncWaitResumeFlags{IORING_CQE_F_MORE};
    while (asyncWaitResumeFlags & IORING_CQE_F_MORE) {
        action(co_await asyncWaiter);
        asyncWaitResumeFlags = asyncWaiter.getAsyncWaitResumeFlags();
    }
}

auto coContext::multipleAcceptDirect(std::move_only_function<auto(std::int32_t)->void> action,
                                     const std::int32_t socketFileDescriptor, sockaddr *const address,
                                     socklen_t *const addressLength, const std::int32_t flags) -> Task<> {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.multipleAcceptDirect(socketFileDescriptor, address, addressLength, flags);
    submissionQueueEntry.addIoPriority(IORING_ACCEPT_POLL_FIRST);

    internal::AsyncWaiter asyncWaiter{submissionQueueEntry};

    std::uint32_t asyncWaitResumeFlags{IORING_CQE_F_MORE};
    while (asyncWaitResumeFlags & IORING_CQE_F_MORE) {
        action(co_await asyncWaiter);
        asyncWaitResumeFlags = asyncWaiter.getAsyncWaitResumeFlags();
    }
}

auto coContext::connect(const std::int32_t socketFileDescriptor, const sockaddr &address, const socklen_t addressLength)
    -> internal::AsyncWaiter {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.connect(socketFileDescriptor, address, addressLength);

    return internal::AsyncWaiter{submissionQueueEntry};
}

auto coContext::shutdown(const std::int32_t socketFileDescriptor, const std::int32_t how) -> internal::AsyncWaiter {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.shutdown(socketFileDescriptor, how);

    return internal::AsyncWaiter{submissionQueueEntry};
}

auto coContext::receive(const std::int32_t socketFileDescriptor, const std::span<std::byte> buffer,
                        const std::int32_t flags) -> internal::AsyncWaiter {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.receive(socketFileDescriptor, buffer, flags);
    submissionQueueEntry.addIoPriority(IORING_RECVSEND_POLL_FIRST);

    return internal::AsyncWaiter{submissionQueueEntry};
}

auto coContext::receive(const std::int32_t socketFileDescriptor, msghdr &message, const std::uint32_t flags)
    -> internal::AsyncWaiter {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.receive(socketFileDescriptor, message, flags);
    submissionQueueEntry.addIoPriority(IORING_RECVSEND_POLL_FIRST);

    return internal::AsyncWaiter{submissionQueueEntry};
}

auto coContext::send(const std::int32_t socketFileDescriptor, const std::span<const std::byte> buffer,
                     const std::int32_t flags) -> internal::AsyncWaiter {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.send(socketFileDescriptor, buffer, flags);
    submissionQueueEntry.addIoPriority(IORING_RECVSEND_POLL_FIRST);

    return internal::AsyncWaiter{submissionQueueEntry};
}

auto coContext::send(const std::int32_t socketFileDescriptor, const std::span<const std::byte> buffer,
                     const std::int32_t flags, const sockaddr &destinationAddress,
                     const socklen_t destinationAddressLength) -> internal::AsyncWaiter {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.send(socketFileDescriptor, buffer, flags, destinationAddress, destinationAddressLength);

    return internal::AsyncWaiter{submissionQueueEntry};
}

auto coContext::send(const std::int32_t socketFileDescriptor, const msghdr &message, const std::uint32_t flags)
    -> internal::AsyncWaiter {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.send(socketFileDescriptor, message, flags);
    submissionQueueEntry.addIoPriority(IORING_RECVSEND_POLL_FIRST);

    return internal::AsyncWaiter{submissionQueueEntry};
}

auto coContext::splice(const std::int32_t inFileDescriptor, const std::int64_t inFileDescriptorOffset,
                       const std::int32_t outFileDescriptor, const std::int64_t outFileDescriptorOffset,
                       const std::uint32_t length, const std::uint32_t flags) -> internal::AsyncWaiter {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.splice(inFileDescriptor, inFileDescriptorOffset, outFileDescriptor, outFileDescriptorOffset,
                                length, flags);

    return internal::AsyncWaiter{submissionQueueEntry};
}

auto coContext::tee(const std::int32_t inFileDescriptor, const std::int32_t outFileDescriptor,
                    const std::uint32_t length, const std::uint32_t flags) -> internal::AsyncWaiter {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.tee(inFileDescriptor, outFileDescriptor, length, flags);

    return internal::AsyncWaiter{submissionQueueEntry};
}

auto coContext::open(const std::string_view path, const std::int32_t flags, const mode_t mode)
    -> internal::AsyncWaiter {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.open(path, flags, mode);

    return internal::AsyncWaiter{submissionQueueEntry};
}

auto coContext::open(const std::int32_t directoryFileDescriptor, const std::string_view path, const std::int32_t flags,
                     const mode_t mode) -> internal::AsyncWaiter {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.open(directoryFileDescriptor, path, flags, mode);

    return internal::AsyncWaiter{submissionQueueEntry};
}

auto coContext::open(const std::int32_t directoryFileDescriptor, const std::string_view path, open_how &openHow)
    -> internal::AsyncWaiter {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.open(directoryFileDescriptor, path, openHow);

    return internal::AsyncWaiter{submissionQueueEntry};
}

auto coContext::openDirect(const std::string_view path, const std::int32_t flags, const mode_t mode)
    -> internal::AsyncWaiter {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.openDirect(path, flags, mode, IORING_FILE_INDEX_ALLOC);

    return internal::AsyncWaiter{submissionQueueEntry};
}

auto coContext::openDirect(const std::int32_t directoryFileDescriptor, const std::string_view path,
                           const std::int32_t flags, const mode_t mode) -> internal::AsyncWaiter {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.openDirect(directoryFileDescriptor, path, flags, mode, IORING_FILE_INDEX_ALLOC);

    return internal::AsyncWaiter{submissionQueueEntry};
}

auto coContext::openDirect(const std::int32_t directoryFileDescriptor, const std::string_view path, open_how &openHow)
    -> internal::AsyncWaiter {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.openDirect(directoryFileDescriptor, path, openHow, IORING_FILE_INDEX_ALLOC);

    return internal::AsyncWaiter{submissionQueueEntry};
}

auto coContext::read(const std::int32_t fileDescriptor, const std::span<std::byte> buffer, const std::uint64_t offset)
    -> internal::AsyncWaiter {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.read(fileDescriptor, buffer, offset);

    return internal::AsyncWaiter{submissionQueueEntry};
}

auto coContext::read(const std::int32_t fileDescriptor, const std::span<const iovec> buffer, const std::uint64_t offset)
    -> internal::AsyncWaiter {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.read(fileDescriptor, buffer, offset);

    return internal::AsyncWaiter{submissionQueueEntry};
}

auto coContext::read(const std::int32_t fileDescriptor, const std::span<const iovec> buffer, const std::uint64_t offset,
                     const std::int32_t flags) -> internal::AsyncWaiter {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.read(fileDescriptor, buffer, offset, flags);

    return internal::AsyncWaiter{submissionQueueEntry};
}

auto coContext::write(const std::int32_t fileDescriptor, const std::span<const std::byte> buffer,
                      const std::uint64_t offset) -> internal::AsyncWaiter {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.write(fileDescriptor, buffer, offset);

    return internal::AsyncWaiter{submissionQueueEntry};
}

auto coContext::write(const std::int32_t fileDescriptor, const std::span<const iovec> buffer,
                      const std::uint64_t offset) -> internal::AsyncWaiter {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.write(fileDescriptor, buffer, offset);

    return internal::AsyncWaiter{submissionQueueEntry};
}

auto coContext::write(const std::int32_t fileDescriptor, const std::span<const iovec> buffer,
                      const std::uint64_t offset, const std::int32_t flags) -> internal::AsyncWaiter {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.write(fileDescriptor, buffer, offset, flags);

    return internal::AsyncWaiter{submissionQueueEntry};
}

auto coContext::syncFile(const std::int32_t fileDescriptor, const bool isSyncMetadata) -> internal::AsyncWaiter {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.syncFile(fileDescriptor, isSyncMetadata ? 0 : IORING_FSYNC_DATASYNC);

    return internal::AsyncWaiter{submissionQueueEntry};
}

auto coContext::syncFile(const std::int32_t fileDescriptor, const std::uint64_t offset, const std::uint32_t length,
                         const std::int32_t flags) -> internal::AsyncWaiter {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.syncFile(fileDescriptor, offset, length, flags);

    return internal::AsyncWaiter{submissionQueueEntry};
}

auto coContext::adviseFile(const std::int32_t fileDescriptor, const std::uint64_t offset, const off_t length,
                           const std::int32_t advice) -> internal::AsyncWaiter {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.adviseFile(fileDescriptor, offset, length, advice);

    return internal::AsyncWaiter{submissionQueueEntry};
}

auto coContext::truncate(const std::int32_t fileDescriptor, const loff_t length) -> internal::AsyncWaiter {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.truncate(fileDescriptor, length);

    return internal::AsyncWaiter{submissionQueueEntry};
}

auto coContext::allocateFile(const std::int32_t fileDescriptor, const std::int32_t mode, const std::uint64_t offset,
                             const std::uint64_t length) -> internal::AsyncWaiter {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.allocateFile(fileDescriptor, mode, offset, length);

    return internal::AsyncWaiter{submissionQueueEntry};
}

auto coContext::getFileStatus(const std::int32_t directoryFileDescriptor, const std::string_view path,
                              const std::int32_t flags, const std::uint32_t mask, struct statx &buffer)
    -> internal::AsyncWaiter {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.getFileStatus(directoryFileDescriptor, path, flags, mask, buffer);

    return internal::AsyncWaiter{submissionQueueEntry};
}

auto coContext::getExtendedAttribute(const std::string_view path, const std::string_view name,
                                     const std::span<char> value) -> internal::AsyncWaiter {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.getExtendedAttribute(path, name, value);

    return internal::AsyncWaiter{submissionQueueEntry};
}

auto coContext::getExtendedAttribute(const std::int32_t fileDescriptor, const std::string_view name,
                                     const std::span<char> value) -> internal::AsyncWaiter {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.getExtendedAttribute(fileDescriptor, name, value);

    return internal::AsyncWaiter{submissionQueueEntry};
}

auto coContext::setExtendedAttribute(const std::string_view path, const std::string_view name,
                                     const std::span<char> value, const std::int32_t flags) -> internal::AsyncWaiter {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.setExtendedAttribute(path, name, value, flags);

    return internal::AsyncWaiter{submissionQueueEntry};
}

auto coContext::setExtendedAttribute(const std::int32_t fileDescriptor, const std::string_view name,
                                     const std::span<char> value, const std::int32_t flags) -> internal::AsyncWaiter {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.setExtendedAttribute(fileDescriptor, name, value, flags);

    return internal::AsyncWaiter{submissionQueueEntry};
}

auto coContext::makeDirectory(const std::string_view path, const mode_t mode) -> internal::AsyncWaiter {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.makeDirectory(path, mode);

    return internal::AsyncWaiter{submissionQueueEntry};
}

auto coContext::makeDirectory(const std::int32_t directoryFileDescriptor, const std::string_view path,
                              const mode_t mode) -> internal::AsyncWaiter {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.makeDirectory(directoryFileDescriptor, path, mode);

    return internal::AsyncWaiter{submissionQueueEntry};
}

auto coContext::rename(const std::string_view oldPath, const std::string_view newPath) -> internal::AsyncWaiter {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.rename(oldPath, newPath);

    return internal::AsyncWaiter{submissionQueueEntry};
}

auto coContext::rename(const std::int32_t oldDirectoryFileDescriptor, const std::string_view oldPath,
                       const std::int32_t newDirectoryFileDescriptor, const std::string_view newPath,
                       const std::uint32_t flags) -> internal::AsyncWaiter {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.rename(oldDirectoryFileDescriptor, oldPath, newDirectoryFileDescriptor, newPath, flags);

    return internal::AsyncWaiter{submissionQueueEntry};
}

auto coContext::link(const std::string_view oldPath, const std::string_view newPath) -> internal::AsyncWaiter {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.link(oldPath, newPath, 0);

    return internal::AsyncWaiter{submissionQueueEntry};
}

auto coContext::link(const std::int32_t oldDirectoryFileDescriptor, const std::string_view oldPath,
                     const std::int32_t newDirectoryFileDescriptor, const std::string_view newPath,
                     const std::int32_t flags) -> internal::AsyncWaiter {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.link(oldDirectoryFileDescriptor, oldPath, newDirectoryFileDescriptor, newPath, flags);

    return internal::AsyncWaiter{submissionQueueEntry};
}

auto coContext::symbolicLink(const std::string_view target, const std::string_view linkPath) -> internal::AsyncWaiter {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.symbolicLink(target, linkPath);

    return internal::AsyncWaiter{submissionQueueEntry};
}

auto coContext::symbolicLink(const std::string_view target, const std::int32_t newDirectoryFileDescriptor,
                             const std::string_view linkPath) -> internal::AsyncWaiter {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.symbolicLink(target, newDirectoryFileDescriptor, linkPath);

    return internal::AsyncWaiter{submissionQueueEntry};
}

auto coContext::unlink(const std::string_view path) -> internal::AsyncWaiter {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.unlink(path, 0);

    return internal::AsyncWaiter{submissionQueueEntry};
}

auto coContext::unlink(const std::int32_t directoryFileDescriptor, const std::string_view path,
                       const std::int32_t flags) -> internal::AsyncWaiter {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.unlink(directoryFileDescriptor, path, flags);

    return internal::AsyncWaiter{submissionQueueEntry};
}

auto coContext::adviseMemory(const std::span<std::byte> buffer, const std::int32_t advice) -> internal::AsyncWaiter {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.adviseMemory(buffer, advice);

    return internal::AsyncWaiter{submissionQueueEntry};
}

auto coContext::wait(const idtype_t idType, const id_t id, siginfo_t *const signalInformation,
                     const std::int32_t options) -> internal::AsyncWaiter {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.wait(idType, id, signalInformation, options, 0);

    return internal::AsyncWaiter{submissionQueueEntry};
}

auto coContext::waitFutex(std::uint32_t &futex, const std::uint64_t value, const std::uint64_t mask,
                          const std::uint32_t flags) -> internal::AsyncWaiter {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.waitFutex(futex, value, mask, flags, 0);

    return internal::AsyncWaiter{submissionQueueEntry};
}

auto coContext::wakeFutex(std::uint32_t &futex, const std::uint64_t value, const std::uint64_t mask,
                          const std::uint32_t flags) -> internal::AsyncWaiter {
    const internal::SubmissionQueueEntry submissionQueueEntry{context.getSubmissionQueueEntry()};
    submissionQueueEntry.wakeFutex(futex, value, mask, flags, 0);

    return internal::AsyncWaiter{submissionQueueEntry};
}

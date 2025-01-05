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

auto coContext::toDirect(const std::span<std::int32_t> fileDescriptors) -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    submission.updateFileDescriptors(fileDescriptors, IORING_FILE_INDEX_ALLOC);

    return internal::AsyncWaiter{submission};
}

auto coContext::installDirect(const std::int32_t directFileDescriptor, const bool isSetCloseOnExecute)
    -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    submission.installDirect(directFileDescriptor, isSetCloseOnExecute ? 0 : IORING_FIXED_FD_NO_CLOEXEC);

    return internal::AsyncWaiter{submission};
}

auto coContext::none() -> internal::Marker { return internal::Marker{}; }

auto coContext::direct() -> internal::Marker { return internal::Marker{IOSQE_FIXED_FILE}; }

auto coContext::timeout(const std::chrono::seconds seconds, const std::chrono::nanoseconds nanoseconds,
                        const ClockSource clockSource) -> internal::Marker {
    return internal::Marker{
        IOSQE_IO_LINK, [seconds, nanoseconds, clockSource] {
            const internal::Submission submission{context.getSubmission()};
            internal::AsyncWaiter asyncWaiter{submission};

            asyncWaiter.setTimeSpecification(std::make_unique<__kernel_timespec>(seconds.count(), nanoseconds.count()));
            submission.linkTimeout(*asyncWaiter.getTimeSpecification(), setClockSource(clockSource));

            spawn([](internal::AsyncWaiter timeoutAsyncWaiter) -> Task<> { co_await timeoutAsyncWaiter; },
                  std::move(asyncWaiter));
        }};
}

auto coContext::cancel(const std::uint64_t taskIdentity) -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    submission.cancel(taskIdentity, 0);

    return internal::AsyncWaiter{submission};
}

auto coContext::cancel(const std::int32_t fileDescriptor, const bool isMatchAll) -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    submission.cancel(fileDescriptor, isMatchAll ? IORING_ASYNC_CANCEL_ALL : 0);

    return internal::AsyncWaiter{submission};
}

auto coContext::cancelAny() -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    submission.cancel(std::uint64_t{}, std::int32_t{IORING_ASYNC_CANCEL_ANY});

    return internal::AsyncWaiter{submission};
}

[[nodiscard]] constexpr auto sleep(const std::chrono::seconds seconds, const std::chrono::nanoseconds nanoseconds,
                                   const std::uint32_t flags) {
    const coContext::internal::Submission submission{context.getSubmission()};
    coContext::internal::AsyncWaiter asyncWaiter{submission};

    asyncWaiter.setTimeSpecification(std::make_unique<__kernel_timespec>(seconds.count(), nanoseconds.count()));
    submission.timeout(*asyncWaiter.getTimeSpecification(), 0, flags);

    return asyncWaiter;
}

auto coContext::sleep(const std::chrono::seconds seconds, const std::chrono::nanoseconds nanoseconds,
                      const ClockSource clockSource) -> internal::AsyncWaiter {
    return ::sleep(seconds, nanoseconds, setClockSource(clockSource));
}

auto coContext::updateSleep(const std::uint64_t taskIdentity, const std::chrono::seconds seconds,
                            const std::chrono::nanoseconds nanoseconds, const ClockSource clockSource)
    -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    internal::AsyncWaiter asyncWaiter{submission};

    asyncWaiter.setTimeSpecification(std::make_unique<__kernel_timespec>(seconds.count(), nanoseconds.count()));
    submission.updateTimeout(taskIdentity, *asyncWaiter.getTimeSpecification(), setClockSource(clockSource));

    return asyncWaiter;
}

auto coContext::multipleSleep(std::move_only_function<auto(std::int32_t)->void> action,
                              const std::chrono::seconds seconds, const std::chrono::nanoseconds nanoseconds,
                              const ClockSource clockSource, internal::Marker marker) -> Task<> {
    internal::AsyncWaiter asyncWaiter{
        ::sleep(seconds, nanoseconds, setClockSource(clockSource) | IORING_TIMEOUT_MULTISHOT) | std::move(marker)};

    do action(co_await asyncWaiter);
    while ((asyncWaiter.getAsyncWaitResumeFlags() & IORING_CQE_F_MORE) != 0);
}

auto coContext::poll(const std::int32_t fileDescriptor, const std::uint32_t mask) -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    submission.poll(fileDescriptor, mask);

    return internal::AsyncWaiter{submission};
}

auto coContext::updatePoll(const std::uint64_t taskIdentity, const std::uint32_t mask) -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    submission.updatePoll(taskIdentity, 0, mask, IORING_POLL_UPDATE_EVENTS);

    return internal::AsyncWaiter{submission};
}

auto coContext::multiplePoll(std::move_only_function<auto(std::int32_t)->void> action,
                             const std::int32_t fileDescriptor, const std::uint32_t mask, internal::Marker marker)
    -> Task<> {
    const internal::Submission submission{context.getSubmission()};
    submission.multiplePoll(fileDescriptor, mask);

    internal::AsyncWaiter asyncWaiter{internal::AsyncWaiter{submission} | std::move(marker)};

    do action(co_await asyncWaiter);
    while ((asyncWaiter.getAsyncWaitResumeFlags() & IORING_CQE_F_MORE) != 0);
}

auto coContext::close(const std::int32_t fileDescriptor) -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    submission.close(fileDescriptor);

    return internal::AsyncWaiter{submission};
}

auto coContext::closeDirect(const std::int32_t directFileDescriptor) -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    submission.closeDirect(directFileDescriptor);

    return internal::AsyncWaiter{submission};
}

auto coContext::socket(const std::int32_t domain, const std::int32_t type, const std::int32_t protocol)
    -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    submission.socket(domain, type, protocol, 0);

    return internal::AsyncWaiter{submission};
}

auto coContext::directSocket(const std::int32_t domain, const std::int32_t type, const std::int32_t protocol)
    -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    submission.directSocket(domain, type, protocol, 0);

    return internal::AsyncWaiter{submission};
}

auto coContext::getSocketOption(const std::int32_t socketFileDescriptor, const std::int32_t level,
                                const std::int32_t optionName, const std::span<std::byte> option)
    -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    submission.socketCommand(SOCKET_URING_OP_GETSOCKOPT, socketFileDescriptor, level, optionName, option);

    return internal::AsyncWaiter{submission};
}

auto coContext::setSocketOption(const std::int32_t socketFileDescriptor, const std::int32_t level,
                                const std::int32_t optionName, const std::span<std::byte> option)
    -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    submission.socketCommand(SOCKET_URING_OP_SETSOCKOPT, socketFileDescriptor, level, optionName, option);

    return internal::AsyncWaiter{submission};
}

auto coContext::getSocketReceiveBufferUnreadDataSize(const std::int32_t socketFileDescriptor) -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    submission.socketCommand(SOCKET_URING_OP_SIOCINQ, socketFileDescriptor, 0, 0, std::span<std::byte>{});

    return internal::AsyncWaiter{submission};
}

auto coContext::getSocketSendBufferUnsentDataSize(const std::int32_t socketFileDescriptor) -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    submission.socketCommand(SOCKET_URING_OP_SIOCOUTQ, socketFileDescriptor, 0, 0, std::span<std::byte>{});

    return internal::AsyncWaiter{submission};
}

auto coContext::discardData(const std::int32_t fileDescriptor, const std::uint64_t offset, const std::uint64_t length)
    -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    submission.discardCommand(fileDescriptor, offset, length);

    return internal::AsyncWaiter{submission};
}

auto coContext::bind(const std::int32_t socketFileDescriptor, sockaddr &address, const socklen_t addressLength)
    -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    submission.bind(socketFileDescriptor, address, addressLength);

    return internal::AsyncWaiter{submission};
}

auto coContext::listen(const std::int32_t socketFileDescriptor, const std::int32_t backlog) -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    submission.listen(socketFileDescriptor, backlog);

    return internal::AsyncWaiter{submission};
}

auto coContext::accept(const std::int32_t socketFileDescriptor, sockaddr *const address, socklen_t *const addressLength,
                       const std::int32_t flags) -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    submission.accept(socketFileDescriptor, address, addressLength, flags);

    submission.addIoPriority(IORING_ACCEPT_POLL_FIRST);

    return internal::AsyncWaiter{submission};
}

auto coContext::acceptDirect(const std::int32_t socketFileDescriptor, sockaddr *const address,
                             socklen_t *const addressLength, const std::int32_t flags) -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    submission.acceptDirect(socketFileDescriptor, address, addressLength, flags, IORING_FILE_INDEX_ALLOC);

    submission.addIoPriority(IORING_ACCEPT_POLL_FIRST);

    return internal::AsyncWaiter{submission};
}

auto coContext::multipleAccept(std::move_only_function<auto(std::int32_t)->void> action,
                               const std::int32_t socketFileDescriptor, sockaddr *const address,
                               socklen_t *const addressLength, const std::int32_t flags, internal::Marker marker)
    -> Task<> {
    const internal::Submission submission{context.getSubmission()};
    submission.multipleAccept(socketFileDescriptor, address, addressLength, flags);

    submission.addIoPriority(IORING_ACCEPT_POLL_FIRST);

    internal::AsyncWaiter asyncWaiter{internal::AsyncWaiter{submission} | std::move(marker)};

    do action(co_await asyncWaiter);
    while ((asyncWaiter.getAsyncWaitResumeFlags() & IORING_CQE_F_MORE) != 0);
}

auto coContext::multipleAcceptDirect(std::move_only_function<auto(std::int32_t)->void> action,
                                     const std::int32_t socketFileDescriptor, sockaddr *const address,
                                     socklen_t *const addressLength, const std::int32_t flags, internal::Marker marker)
    -> Task<> {
    const internal::Submission submission{context.getSubmission()};
    submission.multipleAcceptDirect(socketFileDescriptor, address, addressLength, flags);

    submission.addIoPriority(IORING_ACCEPT_POLL_FIRST);

    internal::AsyncWaiter asyncWaiter{internal::AsyncWaiter{submission} | std::move(marker)};

    do action(co_await asyncWaiter);
    while ((asyncWaiter.getAsyncWaitResumeFlags() & IORING_CQE_F_MORE) != 0);
}

auto coContext::connect(const std::int32_t socketFileDescriptor, const sockaddr &address, const socklen_t addressLength)
    -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    submission.connect(socketFileDescriptor, address, addressLength);

    return internal::AsyncWaiter{submission};
}

auto coContext::shutdown(const std::int32_t socketFileDescriptor, const std::int32_t how) -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    submission.shutdown(socketFileDescriptor, how);

    return internal::AsyncWaiter{submission};
}

auto coContext::receive(const std::int32_t socketFileDescriptor, const std::span<std::byte> buffer,
                        const std::int32_t flags) -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    submission.receive(socketFileDescriptor, buffer, flags);

    submission.addIoPriority(IORING_RECVSEND_POLL_FIRST);

    return internal::AsyncWaiter{submission};
}

auto coContext::receive(const std::int32_t socketFileDescriptor, msghdr &message, const std::uint32_t flags)
    -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    submission.receive(socketFileDescriptor, message, flags);

    submission.addIoPriority(IORING_RECVSEND_POLL_FIRST);

    return internal::AsyncWaiter{submission};
}

auto coContext::multipleReceive(std::move_only_function<auto(std::int32_t, std::span<const std::byte>)->void> action,
                                const std::int32_t socketFileDescriptor, const std::int32_t flags,
                                internal::Marker marker) -> Task<> {
    bool isRestart;
    do {
        isRestart = false;

        const internal::Submission submission{context.getSubmission()};
        submission.multipleReceive(socketFileDescriptor, std::span<std::byte>{}, flags);

        submission.addFlags(IOSQE_BUFFER_SELECT);
        submission.addIoPriority(IORING_RECVSEND_POLL_FIRST);
        submission.setBufferGroup(context.getRingBufferId());

        internal::AsyncWaiter asyncWaiter{internal::AsyncWaiter{submission} | std::move(marker)};

        std::uint32_t asyncWaitResumeFlags;
        do {
            const std::int32_t result{co_await asyncWaiter};
            if (result == -ENOBUFS) {
                isRestart = true;
                context.expandBuffer();

                break;
            }

            asyncWaitResumeFlags = asyncWaiter.getAsyncWaitResumeFlags();

            std::span<const std::byte> data;
            if ((asyncWaitResumeFlags & IORING_CQE_F_BUFFER) != 0) {
                const std::uint32_t bufferId{asyncWaitResumeFlags >> IORING_CQE_BUFFER_SHIFT};
                data = context.readFromBuffer(bufferId, result);

                if ((asyncWaitResumeFlags & IORING_CQE_F_BUF_MORE) == 0) context.markBufferUsed(bufferId);
            }

            action(result, data);
        } while ((asyncWaitResumeFlags & IORING_CQE_F_MORE) != 0);
    } while (isRestart);
}

auto coContext::send(const std::int32_t socketFileDescriptor, const std::span<const std::byte> buffer,
                     const std::int32_t flags) -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    submission.send(socketFileDescriptor, buffer, flags);

    submission.addIoPriority(IORING_RECVSEND_POLL_FIRST);

    return internal::AsyncWaiter{submission};
}

auto coContext::send(const std::int32_t socketFileDescriptor, const std::span<const std::byte> buffer,
                     const std::int32_t flags, const sockaddr &destinationAddress,
                     const socklen_t destinationAddressLength) -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    submission.send(socketFileDescriptor, buffer, flags, destinationAddress, destinationAddressLength);

    submission.addIoPriority(IORING_RECVSEND_POLL_FIRST);

    return internal::AsyncWaiter{submission};
}

auto coContext::send(const std::int32_t socketFileDescriptor, const msghdr &message, const std::uint32_t flags)
    -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    submission.send(socketFileDescriptor, message, flags);

    submission.addIoPriority(IORING_RECVSEND_POLL_FIRST);

    return internal::AsyncWaiter{submission};
}

auto coContext::zeroCopySend(std::move_only_function<auto(std::int32_t)->void> action,
                             const std::int32_t socketFileDescriptor, const std::span<const std::byte> buffer,
                             std::int32_t flags, internal::Marker marker) -> Task<> {
    if ((marker.getFlags() & IOSQE_IO_LINK) != 0) flags |= MSG_WAITALL;

    const internal::Submission submission{context.getSubmission()};
    submission.zeroCopySend(socketFileDescriptor, buffer, flags, IORING_RECVSEND_POLL_FIRST);

    internal::AsyncWaiter asyncWaiter{internal::AsyncWaiter{submission} | std::move(marker)};

    const std::int32_t result{co_await asyncWaiter};
    if ((asyncWaiter.getAsyncWaitResumeFlags() & IORING_CQE_F_MORE) != 0) co_await asyncWaiter;

    action(result);
}

auto coContext::zeroCopySend(std::move_only_function<auto(std::int32_t)->void> action,
                             const std::int32_t socketFileDescriptor, const msghdr &message, std::int32_t flags,
                             internal::Marker marker) -> Task<> {
    if ((marker.getFlags() & IOSQE_IO_LINK) != 0) flags |= MSG_WAITALL;

    const internal::Submission submission{context.getSubmission()};
    submission.zeroCopySend(socketFileDescriptor, message, flags);

    submission.addIoPriority(IORING_RECVSEND_POLL_FIRST);

    internal::AsyncWaiter asyncWaiter{internal::AsyncWaiter{submission} | std::move(marker)};

    const std::int32_t result{co_await asyncWaiter};
    if ((asyncWaiter.getAsyncWaitResumeFlags() & IORING_CQE_F_MORE) != 0) co_await asyncWaiter;

    action(result);
}

auto coContext::splice(const std::int32_t inFileDescriptor, const std::int64_t inFileDescriptorOffset,
                       const std::int32_t outFileDescriptor, const std::int64_t outFileDescriptorOffset,
                       const std::uint32_t length, const std::uint32_t flags) -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    submission.splice(inFileDescriptor, inFileDescriptorOffset, outFileDescriptor, outFileDescriptorOffset, length,
                      flags);

    return internal::AsyncWaiter{submission};
}

auto coContext::tee(const std::int32_t inFileDescriptor, const std::int32_t outFileDescriptor,
                    const std::uint32_t length, const std::uint32_t flags) -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    submission.tee(inFileDescriptor, outFileDescriptor, length, flags);

    return internal::AsyncWaiter{submission};
}

auto coContext::open(const std::string_view path, const std::int32_t flags, const mode_t mode)
    -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    submission.open(path, flags, mode);

    return internal::AsyncWaiter{submission};
}

auto coContext::open(const std::int32_t directoryFileDescriptor, const std::string_view path, const std::int32_t flags,
                     const mode_t mode) -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    submission.open(directoryFileDescriptor, path, flags, mode);

    return internal::AsyncWaiter{submission};
}

auto coContext::open(const std::int32_t directoryFileDescriptor, const std::string_view path, open_how &openHow)
    -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    submission.open(directoryFileDescriptor, path, openHow);

    return internal::AsyncWaiter{submission};
}

auto coContext::openDirect(const std::string_view path, const std::int32_t flags, const mode_t mode)
    -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    submission.openDirect(path, flags, mode, IORING_FILE_INDEX_ALLOC);

    return internal::AsyncWaiter{submission};
}

auto coContext::openDirect(const std::int32_t directoryFileDescriptor, const std::string_view path,
                           const std::int32_t flags, const mode_t mode) -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    submission.openDirect(directoryFileDescriptor, path, flags, mode, IORING_FILE_INDEX_ALLOC);

    return internal::AsyncWaiter{submission};
}

auto coContext::openDirect(const std::int32_t directoryFileDescriptor, const std::string_view path, open_how &openHow)
    -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    submission.openDirect(directoryFileDescriptor, path, openHow, IORING_FILE_INDEX_ALLOC);

    return internal::AsyncWaiter{submission};
}

auto coContext::read(const std::int32_t fileDescriptor, const std::span<std::byte> buffer, const std::uint64_t offset)
    -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    submission.read(fileDescriptor, buffer, offset);

    return internal::AsyncWaiter{submission};
}

auto coContext::read(const std::int32_t fileDescriptor, const std::span<const iovec> buffer, const std::uint64_t offset)
    -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    submission.read(fileDescriptor, buffer, offset);

    return internal::AsyncWaiter{submission};
}

auto coContext::read(const std::int32_t fileDescriptor, const std::span<const iovec> buffer, const std::uint64_t offset,
                     const std::int32_t flags) -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    submission.read(fileDescriptor, buffer, offset, flags);

    return internal::AsyncWaiter{submission};
}

auto coContext::multipleRead(std::move_only_function<auto(std::int32_t, std::span<const std::byte>)->void> action,
                             const std::int32_t fileDescriptor, const std::int32_t offset, internal::Marker marker)
    -> Task<> {
    bool isRestart;
    do {
        isRestart = false;

        const internal::Submission submission{context.getSubmission()};
        submission.multipleRead(fileDescriptor, 0, offset, context.getRingBufferId());

        internal::AsyncWaiter asyncWaiter{internal::AsyncWaiter{submission} | std::move(marker)};

        std::uint32_t asyncWaitResumeFlags;
        do {
            const std::int32_t result{co_await asyncWaiter};
            if (result == -ENOBUFS) {
                isRestart = true;
                context.expandBuffer();

                break;
            }

            asyncWaitResumeFlags = asyncWaiter.getAsyncWaitResumeFlags();

            std::span<const std::byte> data;
            if ((asyncWaitResumeFlags & IORING_CQE_F_BUFFER) != 0) {
                const std::uint32_t bufferId{asyncWaitResumeFlags >> IORING_CQE_BUFFER_SHIFT};
                data = context.readFromBuffer(bufferId, result);

                if ((asyncWaitResumeFlags & IORING_CQE_F_BUF_MORE) == 0) context.markBufferUsed(bufferId);
            }

            action(result, data);
        } while ((asyncWaitResumeFlags & IORING_CQE_F_MORE) != 0);
    } while (isRestart);
}

auto coContext::write(const std::int32_t fileDescriptor, const std::span<const std::byte> buffer,
                      const std::uint64_t offset) -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    submission.write(fileDescriptor, buffer, offset);

    return internal::AsyncWaiter{submission};
}

auto coContext::write(const std::int32_t fileDescriptor, const std::span<const iovec> buffer,
                      const std::uint64_t offset) -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    submission.write(fileDescriptor, buffer, offset);

    return internal::AsyncWaiter{submission};
}

auto coContext::write(const std::int32_t fileDescriptor, const std::span<const iovec> buffer,
                      const std::uint64_t offset, const std::int32_t flags) -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    submission.write(fileDescriptor, buffer, offset, flags);

    return internal::AsyncWaiter{submission};
}

auto coContext::syncFile(const std::int32_t fileDescriptor, const bool isSyncMetadata) -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    submission.syncFile(fileDescriptor, isSyncMetadata ? 0 : IORING_FSYNC_DATASYNC);

    return internal::AsyncWaiter{submission};
}

auto coContext::syncFile(const std::int32_t fileDescriptor, const std::uint64_t offset, const std::uint32_t length,
                         const std::int32_t flags) -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    submission.syncFile(fileDescriptor, offset, length, flags);

    return internal::AsyncWaiter{submission};
}

auto coContext::adviseFile(const std::int32_t fileDescriptor, const std::uint64_t offset, const off_t length,
                           const std::int32_t advice) -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    submission.adviseFile(fileDescriptor, offset, length, advice);

    return internal::AsyncWaiter{submission};
}

auto coContext::truncate(const std::int32_t fileDescriptor, const loff_t length) -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    submission.truncate(fileDescriptor, length);

    return internal::AsyncWaiter{submission};
}

auto coContext::allocateFile(const std::int32_t fileDescriptor, const std::int32_t mode, const std::uint64_t offset,
                             const std::uint64_t length) -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    submission.allocateFile(fileDescriptor, mode, offset, length);

    return internal::AsyncWaiter{submission};
}

auto coContext::status(const std::int32_t directoryFileDescriptor, const std::string_view path,
                       const std::int32_t flags, const std::uint32_t mask, struct statx &buffer)
    -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    submission.status(directoryFileDescriptor, path, flags, mask, buffer);

    return internal::AsyncWaiter{submission};
}

auto coContext::getExtendedAttribute(const std::string_view path, const std::string_view name,
                                     const std::span<char> value) -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    submission.getExtendedAttribute(path, name, value);

    return internal::AsyncWaiter{submission};
}

auto coContext::getExtendedAttribute(const std::int32_t fileDescriptor, const std::string_view name,
                                     const std::span<char> value) -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    submission.getExtendedAttribute(fileDescriptor, name, value);

    return internal::AsyncWaiter{submission};
}

auto coContext::setExtendedAttribute(const std::string_view path, const std::string_view name,
                                     const std::span<char> value, const std::int32_t flags) -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    submission.setExtendedAttribute(path, name, value, flags);

    return internal::AsyncWaiter{submission};
}

auto coContext::setExtendedAttribute(const std::int32_t fileDescriptor, const std::string_view name,
                                     const std::span<char> value, const std::int32_t flags) -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    submission.setExtendedAttribute(fileDescriptor, name, value, flags);

    return internal::AsyncWaiter{submission};
}

auto coContext::makeDirectory(const std::string_view path, const mode_t mode) -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    submission.makeDirectory(path, mode);

    return internal::AsyncWaiter{submission};
}

auto coContext::makeDirectory(const std::int32_t directoryFileDescriptor, const std::string_view path,
                              const mode_t mode) -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    submission.makeDirectory(directoryFileDescriptor, path, mode);

    return internal::AsyncWaiter{submission};
}

auto coContext::rename(const std::string_view oldPath, const std::string_view newPath) -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    submission.rename(oldPath, newPath);

    return internal::AsyncWaiter{submission};
}

auto coContext::rename(const std::int32_t oldDirectoryFileDescriptor, const std::string_view oldPath,
                       const std::int32_t newDirectoryFileDescriptor, const std::string_view newPath,
                       const std::uint32_t flags) -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    submission.rename(oldDirectoryFileDescriptor, oldPath, newDirectoryFileDescriptor, newPath, flags);

    return internal::AsyncWaiter{submission};
}

auto coContext::link(const std::string_view oldPath, const std::string_view newPath) -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    submission.link(oldPath, newPath, 0);

    return internal::AsyncWaiter{submission};
}

auto coContext::link(const std::int32_t oldDirectoryFileDescriptor, const std::string_view oldPath,
                     const std::int32_t newDirectoryFileDescriptor, const std::string_view newPath,
                     const std::int32_t flags) -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    submission.link(oldDirectoryFileDescriptor, oldPath, newDirectoryFileDescriptor, newPath, flags);

    return internal::AsyncWaiter{submission};
}

auto coContext::symbolicLink(const std::string_view target, const std::string_view linkPath) -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    submission.symbolicLink(target, linkPath);

    return internal::AsyncWaiter{submission};
}

auto coContext::symbolicLink(const std::string_view target, const std::int32_t newDirectoryFileDescriptor,
                             const std::string_view linkPath) -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    submission.symbolicLink(target, newDirectoryFileDescriptor, linkPath);

    return internal::AsyncWaiter{submission};
}

auto coContext::unlink(const std::string_view path) -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    submission.unlink(path, 0);

    return internal::AsyncWaiter{submission};
}

auto coContext::unlink(const std::int32_t directoryFileDescriptor, const std::string_view path,
                       const std::int32_t flags) -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    submission.unlink(directoryFileDescriptor, path, flags);

    return internal::AsyncWaiter{submission};
}

auto coContext::adviseMemory(const std::span<std::byte> buffer, const std::int32_t advice) -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    submission.adviseMemory(buffer, advice);

    return internal::AsyncWaiter{submission};
}

auto coContext::wait(const idtype_t idType, const id_t id, siginfo_t *const signalInformation,
                     const std::int32_t options) -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    submission.wait(idType, id, signalInformation, options, 0);

    return internal::AsyncWaiter{submission};
}

auto coContext::waitFutex(std::uint32_t &futex, const std::uint64_t value, const std::uint64_t mask,
                          const std::uint32_t flags) -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    submission.waitFutex(futex, value, mask, flags, 0);

    return internal::AsyncWaiter{submission};
}

auto coContext::waitFutex(const std::span<futex_waitv> vectorizedFutexs) -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    submission.waitFutex(vectorizedFutexs, 0);

    return internal::AsyncWaiter{submission};
}

auto coContext::wakeFutex(std::uint32_t &futex, const std::uint64_t value, const std::uint64_t mask,
                          const std::uint32_t flags) -> internal::AsyncWaiter {
    const internal::Submission submission{context.getSubmission()};
    submission.wakeFutex(futex, value, mask, flags, 0);

    return internal::AsyncWaiter{submission};
}

#include "coContext/coContext.hpp"

#include "context/Context.hpp"
#include "log/Exception.hpp"

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

    [[nodiscard]] constexpr auto rawSleep(const std::chrono::seconds seconds,
                                          const std::chrono::nanoseconds nanoseconds, const std::uint32_t flags) {
        auto timeSpecification{std::make_unique<__kernel_timespec>(seconds.count(), nanoseconds.count())};

        coContext::internal::AsyncWaiter asyncWaiter{
            coContext::internal::Submission::timeout(context.getSubmission(), timeSpecification.get(), 0, flags)};
        asyncWaiter.setTimeSpecification(std::move(timeSpecification));

        return asyncWaiter;
    }
}    // namespace

auto coContext::internal::spawn(Coroutine coroutine) -> void { context.spawn(std::move(coroutine)); }

auto coContext::run() -> void { context.run(); }

auto coContext::stop() -> void { context.stop(); }

auto coContext::syncCancel(const std::uint64_t taskId, const std::chrono::seconds seconds,
                           const std::chrono::nanoseconds nanoseconds) -> std::int32_t {
    return context.syncCancel(taskId, 0, __kernel_timespec{seconds.count(), nanoseconds.count()});
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

auto coContext::none() noexcept -> internal::Marker { return internal::Marker{}; }

auto coContext::direct() noexcept -> internal::Marker { return internal::Marker{IOSQE_FIXED_FILE}; }

auto coContext::timeout(const std::chrono::seconds seconds, const std::chrono::nanoseconds nanoseconds,
                        const ClockSource clockSource) -> internal::Marker {
    return internal::Marker{IOSQE_IO_LINK, [seconds, nanoseconds, clockSource] constexpr {
                                spawn([seconds, nanoseconds, clockSource] -> Task<> {
                                    auto timeSpecification{
                                        std::make_unique<__kernel_timespec>(seconds.count(), nanoseconds.count())};

                                    internal::AsyncWaiter asyncWaiter{internal::Submission::linkTimeout(
                                        context.getSubmission(), timeSpecification.get(), setClockSource(clockSource))};
                                    asyncWaiter.setTimeSpecification(std::move(timeSpecification));

                                    co_await asyncWaiter;
                                });
                            }};
}

auto coContext::noOperation() -> internal::AsyncWaiter {
    return internal::AsyncWaiter{internal::Submission::noOperation(context.getSubmission())};
}

auto coContext::cancel(const std::uint64_t taskId) -> internal::AsyncWaiter {
    return internal::AsyncWaiter{internal::Submission::cancel(context.getSubmission(), taskId, 0)};
}

auto coContext::cancel(const std::int32_t fileDescriptor, const bool isMatchAll) -> internal::AsyncWaiter {
    return internal::AsyncWaiter{internal::Submission::cancel(context.getSubmission(), fileDescriptor,
                                                              isMatchAll ? IORING_ASYNC_CANCEL_ALL : 0)};
}

auto coContext::cancelAny() -> internal::AsyncWaiter {
    return internal::AsyncWaiter{
        internal::Submission::cancel(context.getSubmission(), std::uint64_t{}, std::int32_t{IORING_ASYNC_CANCEL_ANY})};
}

auto coContext::sleep(const std::chrono::seconds seconds, const std::chrono::nanoseconds nanoseconds,
                      const ClockSource clockSource) -> internal::AsyncWaiter {
    return rawSleep(seconds, nanoseconds, setClockSource(clockSource));
}

auto coContext::updateSleep(const std::uint64_t taskId, const std::chrono::seconds seconds,
                            const std::chrono::nanoseconds nanoseconds, const ClockSource clockSource)
    -> internal::AsyncWaiter {
    auto timeSpecification{std::make_unique<__kernel_timespec>(seconds.count(), nanoseconds.count())};

    internal::AsyncWaiter asyncWaiter{internal::Submission::updateTimeout(
        context.getSubmission(), taskId, timeSpecification.get(), setClockSource(clockSource))};
    asyncWaiter.setTimeSpecification(std::move(timeSpecification));

    return asyncWaiter;
}

auto coContext::multipleSleep(std::move_only_function<auto(std::int32_t)->void> action,
                              const std::chrono::seconds seconds, const std::chrono::nanoseconds nanoseconds,
                              const ClockSource clockSource, const internal::Marker marker) -> Task<> {
    internal::AsyncWaiter asyncWaiter{
        rawSleep(seconds, nanoseconds, setClockSource(clockSource) | IORING_TIMEOUT_MULTISHOT) | marker};

    do action(co_await asyncWaiter);
    while ((asyncWaiter.getResumeFlags() & IORING_CQE_F_MORE) != 0);
}

auto coContext::poll(const std::int32_t fileDescriptor, const std::uint32_t mask) -> internal::AsyncWaiter {
    return internal::AsyncWaiter{internal::Submission::poll(context.getSubmission(), fileDescriptor, mask)};
}

auto coContext::updatePoll(const std::uint64_t taskId, const std::uint32_t mask) -> internal::AsyncWaiter {
    return internal::AsyncWaiter{
        internal::Submission::updatePoll(context.getSubmission(), taskId, 0, mask, IORING_POLL_UPDATE_EVENTS)};
}

auto coContext::multiplePoll(std::move_only_function<auto(std::int32_t)->void> action,
                             const std::int32_t fileDescriptor, const std::uint32_t mask, const internal::Marker marker)
    -> Task<> {
    internal::AsyncWaiter asyncWaiter{
        internal::AsyncWaiter{internal::Submission::multiplePoll(context.getSubmission(), fileDescriptor, mask)} |
        marker};

    do action(co_await asyncWaiter);
    while ((asyncWaiter.getResumeFlags() & IORING_CQE_F_MORE) != 0);
}

auto coContext::toDirect(const std::span<std::int32_t> fileDescriptors) -> internal::AsyncWaiter {
    return internal::AsyncWaiter{
        internal::Submission::updateFileDescriptors(context.getSubmission(), fileDescriptors, IORING_FILE_INDEX_ALLOC)};
}

auto coContext::installDirect(const std::int32_t directFileDescriptor, const bool isCloseOnExecute)
    -> internal::AsyncWaiter {
    return internal::AsyncWaiter{internal::Submission::installDirect(
        context.getSubmission(), directFileDescriptor, isCloseOnExecute ? 0 : IORING_FIXED_FD_NO_CLOEXEC)};
}

auto coContext::close(const std::int32_t fileDescriptor) -> internal::AsyncWaiter {
    return internal::AsyncWaiter{internal::Submission::close(context.getSubmission(), fileDescriptor)};
}

auto coContext::closeDirect(const std::int32_t directFileDescriptor) -> internal::AsyncWaiter {
    return internal::AsyncWaiter{internal::Submission::closeDirect(context.getSubmission(), directFileDescriptor)};
}

auto coContext::socket(const std::int32_t domain, const std::int32_t type, const std::int32_t protocol)
    -> internal::AsyncWaiter {
    return internal::AsyncWaiter{internal::Submission::socket(context.getSubmission(), domain, type, protocol, 0)};
}

auto coContext::directSocket(const std::int32_t domain, const std::int32_t type, const std::int32_t protocol)
    -> internal::AsyncWaiter {
    return internal::AsyncWaiter{
        internal::Submission::directSocket(context.getSubmission(), domain, type, protocol, 0)};
}

auto coContext::getSocketOption(const std::int32_t socketFileDescriptor, const std::int32_t level,
                                const std::int32_t optionName, const std::span<std::byte> option)
    -> internal::AsyncWaiter {
    return internal::AsyncWaiter{internal::Submission::socketCommand(
        context.getSubmission(), SOCKET_URING_OP_GETSOCKOPT, socketFileDescriptor, level, optionName, option)};
}

auto coContext::setSocketOption(const std::int32_t socketFileDescriptor, const std::int32_t level,
                                const std::int32_t optionName, const std::span<std::byte> option)
    -> internal::AsyncWaiter {
    return internal::AsyncWaiter{internal::Submission::socketCommand(
        context.getSubmission(), SOCKET_URING_OP_SETSOCKOPT, socketFileDescriptor, level, optionName, option)};
}

auto coContext::getSocketReceiveBufferUnreadDataSize(const std::int32_t socketFileDescriptor) -> internal::AsyncWaiter {
    return internal::AsyncWaiter{internal::Submission::socketCommand(
        context.getSubmission(), SOCKET_URING_OP_SIOCINQ, socketFileDescriptor, 0, 0, std::span<std::byte>{})};
}

auto coContext::getSocketSendBufferUnsentDataSize(const std::int32_t socketFileDescriptor) -> internal::AsyncWaiter {
    return internal::AsyncWaiter{internal::Submission::socketCommand(
        context.getSubmission(), SOCKET_URING_OP_SIOCOUTQ, socketFileDescriptor, 0, 0, std::span<std::byte>{})};
}

auto coContext::discardData(const std::int32_t fileDescriptor, const std::uint64_t offset, const std::uint64_t length)
    -> internal::AsyncWaiter {
    return internal::AsyncWaiter{
        internal::Submission::discardCommand(context.getSubmission(), fileDescriptor, offset, length)};
}

auto coContext::bind(const std::int32_t socketFileDescriptor, sockaddr &address, const socklen_t addressLength)
    -> internal::AsyncWaiter {
    return internal::AsyncWaiter{
        internal::Submission::bind(context.getSubmission(), socketFileDescriptor, address, addressLength)};
}

auto coContext::listen(const std::int32_t socketFileDescriptor, const std::int32_t backlog) -> internal::AsyncWaiter {
    return internal::AsyncWaiter{internal::Submission::listen(context.getSubmission(), socketFileDescriptor, backlog)};
}

auto coContext::accept(const std::int32_t socketFileDescriptor, sockaddr *const address, socklen_t *const addressLength,
                       const std::int32_t flags) -> internal::AsyncWaiter {
    const internal::Submission submission{
        internal::Submission::accept(context.getSubmission(), socketFileDescriptor, address, addressLength, flags)};
    submission.addIoPriority(IORING_ACCEPT_POLL_FIRST);

    return internal::AsyncWaiter{submission};
}

auto coContext::acceptDirect(const std::int32_t socketFileDescriptor, sockaddr *const address,
                             socklen_t *const addressLength, const std::int32_t flags) -> internal::AsyncWaiter {
    const internal::Submission submission{internal::Submission::acceptDirect(
        context.getSubmission(), socketFileDescriptor, address, addressLength, flags, IORING_FILE_INDEX_ALLOC)};
    submission.addIoPriority(IORING_ACCEPT_POLL_FIRST);

    return internal::AsyncWaiter{submission};
}

auto coContext::multipleAccept(std::move_only_function<auto(std::int32_t)->void> action,
                               const std::int32_t socketFileDescriptor, sockaddr *const address,
                               socklen_t *const addressLength, const std::int32_t flags, const internal::Marker marker)
    -> Task<> {
    const internal::Submission submission{internal::Submission::multipleAccept(
        context.getSubmission(), socketFileDescriptor, address, addressLength, flags)};
    submission.addIoPriority(IORING_ACCEPT_POLL_FIRST);

    internal::AsyncWaiter asyncWaiter{internal::AsyncWaiter{submission} | marker};

    do action(co_await asyncWaiter);
    while ((asyncWaiter.getResumeFlags() & IORING_CQE_F_MORE) != 0);
}

auto coContext::multipleAcceptDirect(std::move_only_function<auto(std::int32_t)->void> action,
                                     const std::int32_t socketFileDescriptor, sockaddr *const address,
                                     socklen_t *const addressLength, const std::int32_t flags,
                                     const internal::Marker marker) -> Task<> {
    const internal::Submission submission{internal::Submission::multipleAcceptDirect(
        context.getSubmission(), socketFileDescriptor, address, addressLength, flags)};
    submission.addIoPriority(IORING_ACCEPT_POLL_FIRST);

    internal::AsyncWaiter asyncWaiter{internal::AsyncWaiter{submission} | marker};

    do action(co_await asyncWaiter);
    while ((asyncWaiter.getResumeFlags() & IORING_CQE_F_MORE) != 0);
}

auto coContext::connect(const std::int32_t socketFileDescriptor, const sockaddr &address, const socklen_t addressLength)
    -> internal::AsyncWaiter {
    return internal::AsyncWaiter{
        internal::Submission::connect(context.getSubmission(), socketFileDescriptor, address, addressLength)};
}

auto coContext::shutdown(const std::int32_t socketFileDescriptor, const std::int32_t how) -> internal::AsyncWaiter {
    return internal::AsyncWaiter{internal::Submission::shutdown(context.getSubmission(), socketFileDescriptor, how)};
}

auto coContext::receive(const std::int32_t socketFileDescriptor, const std::span<std::byte> buffer,
                        const std::int32_t flags) -> internal::AsyncWaiter {
    const internal::Submission submission{
        internal::Submission::receive(context.getSubmission(), socketFileDescriptor, buffer, flags)};
    submission.addIoPriority(IORING_RECVSEND_POLL_FIRST);

    return internal::AsyncWaiter{submission};
}

auto coContext::receive(const std::int32_t socketFileDescriptor, msghdr &message, const std::uint32_t flags)
    -> internal::AsyncWaiter {
    const internal::Submission submission{
        internal::Submission::receive(context.getSubmission(), socketFileDescriptor, message, flags)};
    submission.addIoPriority(IORING_RECVSEND_POLL_FIRST);

    return internal::AsyncWaiter{submission};
}

auto coContext::multipleReceive(std::move_only_function<auto(std::int32_t, std::span<const std::byte>)->void> action,
                                const std::int32_t socketFileDescriptor, const std::int32_t flags,
                                const internal::Marker marker) -> Task<> {
    bool isRestart;
    do {
        isRestart = false;

        const internal::Submission submission{internal::Submission::multipleReceive(
            context.getSubmission(), socketFileDescriptor, std::span<std::byte>{}, flags)};
        submission.addFlags(IOSQE_BUFFER_SELECT);
        submission.addIoPriority(IORING_RECVSEND_POLL_FIRST);
        submission.setBufferGroup(context.getBufferRing().getId());

        internal::AsyncWaiter asyncWaiter{internal::AsyncWaiter{submission} | marker};

        std::uint32_t resumeFlags;
        do {
            const std::int32_t result{co_await asyncWaiter};
            if (result == -ENOBUFS) {
                logger::write(Log{
                    Log::Level::warn,
                    std::pmr::string{std::error_code{std::abs(result), std::generic_category()}.message(),
                                     internal::getSyncMemoryResource()}
                });

                try {
                    context.getBufferRing().expandBuffer();
                } catch (internal::Exception &exception) { logger::write(std::move(exception.getLog())); }

                isRestart = true;

                break;
            }

            resumeFlags = asyncWaiter.getResumeFlags();

            std::span<const std::byte> data;
            if ((resumeFlags & IORING_CQE_F_BUFFER) != 0) {
                const std::uint32_t bufferId{resumeFlags >> IORING_CQE_BUFFER_SHIFT};
                data = context.getBufferRing().readData(bufferId, result);

                if ((resumeFlags & IORING_CQE_F_BUF_MORE) == 0) context.getBufferRing().markBufferUsed(bufferId);
            }

            action(result, data);
        } while ((resumeFlags & IORING_CQE_F_MORE) != 0);
    } while (isRestart);
}

auto coContext::send(const std::int32_t socketFileDescriptor, const std::span<const std::byte> buffer,
                     const std::int32_t flags) -> internal::AsyncWaiter {
    const internal::Submission submission{
        internal::Submission::send(context.getSubmission(), socketFileDescriptor, buffer, flags)};
    submission.addIoPriority(IORING_RECVSEND_POLL_FIRST);

    return internal::AsyncWaiter{submission};
}

auto coContext::send(const std::int32_t socketFileDescriptor, const std::span<const std::byte> buffer,
                     const std::int32_t flags, const sockaddr &destinationAddress,
                     const socklen_t destinationAddressLength) -> internal::AsyncWaiter {
    const internal::Submission submission{internal::Submission::send(
        context.getSubmission(), socketFileDescriptor, buffer, flags, destinationAddress, destinationAddressLength)};
    submission.addIoPriority(IORING_RECVSEND_POLL_FIRST);

    return internal::AsyncWaiter{submission};
}

auto coContext::send(const std::int32_t socketFileDescriptor, const msghdr &message, const std::uint32_t flags)
    -> internal::AsyncWaiter {
    const internal::Submission submission{
        internal::Submission::send(context.getSubmission(), socketFileDescriptor, message, flags)};
    submission.addIoPriority(IORING_RECVSEND_POLL_FIRST);

    return internal::AsyncWaiter{submission};
}

auto coContext::zeroCopySend(std::move_only_function<auto(std::int32_t)->void> action,
                             const std::int32_t socketFileDescriptor, const std::span<const std::byte> buffer,
                             std::int32_t flags, const internal::Marker marker) -> Task<> {
    if ((marker.getFlags() & IOSQE_IO_LINK) != 0) flags |= MSG_WAITALL;

    internal::AsyncWaiter asyncWaiter{
        internal::AsyncWaiter{internal::Submission::zeroCopySend(context.getSubmission(), socketFileDescriptor, buffer,
                                                                 flags, IORING_RECVSEND_POLL_FIRST)} |
        marker};

    const std::int32_t result{co_await asyncWaiter};
    if ((asyncWaiter.getResumeFlags() & IORING_CQE_F_MORE) != 0) co_await asyncWaiter;
    else {
        logger::write(Log{
            Log::Level::warn, std::pmr::string{"no notification", internal::getSyncMemoryResource()}
        });
    }

    action(result);
}

auto coContext::zeroCopySend(std::move_only_function<auto(std::int32_t)->void> action,
                             const std::int32_t socketFileDescriptor, const msghdr &message, std::int32_t flags,
                             const internal::Marker marker) -> Task<> {
    if ((marker.getFlags() & IOSQE_IO_LINK) != 0) flags |= MSG_WAITALL;

    const internal::Submission submission{
        internal::Submission::zeroCopySend(context.getSubmission(), socketFileDescriptor, message, flags)};
    submission.addIoPriority(IORING_RECVSEND_POLL_FIRST);

    internal::AsyncWaiter asyncWaiter{internal::AsyncWaiter{submission} | marker};

    const std::int32_t result{co_await asyncWaiter};
    if ((asyncWaiter.getResumeFlags() & IORING_CQE_F_MORE) != 0) co_await asyncWaiter;
    else {
        logger::write(Log{
            Log::Level::warn, std::pmr::string{"no notification", internal::getSyncMemoryResource()}
        });
    }

    action(result);
}

auto coContext::splice(const std::int32_t inFileDescriptor, const std::int64_t inOffset,
                       const std::int32_t outFileDescriptor, const std::int64_t outOffset, const std::uint32_t length,
                       const std::uint32_t flags) -> internal::AsyncWaiter {
    return internal::AsyncWaiter{internal::Submission::splice(context.getSubmission(), inFileDescriptor, inOffset,
                                                              outFileDescriptor, outOffset, length, flags)};
}

auto coContext::tee(const std::int32_t inFileDescriptor, const std::int32_t outFileDescriptor,
                    const std::uint32_t length, const std::uint32_t flags) -> internal::AsyncWaiter {
    return internal::AsyncWaiter{
        internal::Submission::tee(context.getSubmission(), inFileDescriptor, outFileDescriptor, length, flags)};
}

auto coContext::open(const std::filesystem::path &path, const std::int32_t flags, const mode_t mode)
    -> internal::AsyncWaiter {
    return internal::AsyncWaiter{internal::Submission::open(context.getSubmission(), path, flags, mode)};
}

auto coContext::open(const std::int32_t directoryFileDescriptor, const std::filesystem::path &path,
                     const std::int32_t flags, const mode_t mode) -> internal::AsyncWaiter {
    return internal::AsyncWaiter{
        internal::Submission::open(context.getSubmission(), directoryFileDescriptor, path, flags, mode)};
}

auto coContext::open(const std::int32_t directoryFileDescriptor, const std::filesystem::path &path, open_how &openHow)
    -> internal::AsyncWaiter {
    return internal::AsyncWaiter{
        internal::Submission::open(context.getSubmission(), directoryFileDescriptor, path, openHow)};
}

auto coContext::openDirect(const std::filesystem::path &path, const std::int32_t flags, const mode_t mode)
    -> internal::AsyncWaiter {
    return internal::AsyncWaiter{
        internal::Submission::openDirect(context.getSubmission(), path, flags, mode, IORING_FILE_INDEX_ALLOC)};
}

auto coContext::openDirect(const std::int32_t directoryFileDescriptor, const std::filesystem::path &path,
                           const std::int32_t flags, const mode_t mode) -> internal::AsyncWaiter {
    return internal::AsyncWaiter{internal::Submission::openDirect(context.getSubmission(), directoryFileDescriptor,
                                                                  path, flags, mode, IORING_FILE_INDEX_ALLOC)};
}

auto coContext::openDirect(const std::int32_t directoryFileDescriptor, const std::filesystem::path &path,
                           open_how &openHow) -> internal::AsyncWaiter {
    return internal::AsyncWaiter{internal::Submission::openDirect(context.getSubmission(), directoryFileDescriptor,
                                                                  path, openHow, IORING_FILE_INDEX_ALLOC)};
}

auto coContext::read(const std::int32_t fileDescriptor, const std::span<std::byte> buffer, const std::uint64_t offset)
    -> internal::AsyncWaiter {
    return internal::AsyncWaiter{internal::Submission::read(context.getSubmission(), fileDescriptor, buffer, offset)};
}

auto coContext::read(const std::int32_t fileDescriptor, const std::span<const iovec> buffer, const std::uint64_t offset)
    -> internal::AsyncWaiter {
    return internal::AsyncWaiter{internal::Submission::read(context.getSubmission(), fileDescriptor, buffer, offset)};
}

auto coContext::read(const std::int32_t fileDescriptor, const std::span<const iovec> buffer, const std::uint64_t offset,
                     const std::int32_t flags) -> internal::AsyncWaiter {
    return internal::AsyncWaiter{
        internal::Submission::read(context.getSubmission(), fileDescriptor, buffer, offset, flags)};
}

auto coContext::multipleRead(std::move_only_function<auto(std::int32_t, std::span<const std::byte>)->void> action,
                             const std::int32_t fileDescriptor, const std::int32_t offset,
                             const internal::Marker marker) -> Task<> {
    bool isRestart;
    do {
        isRestart = false;

        internal::AsyncWaiter asyncWaiter{
            internal::AsyncWaiter{internal::Submission::multipleRead(context.getSubmission(), fileDescriptor, 0, offset,
                                                                     context.getBufferRing().getId())} |
            marker};

        std::uint32_t resumeFlags;
        do {
            const std::int32_t result{co_await asyncWaiter};
            if (result == -ENOBUFS) {
                logger::write(Log{
                    Log::Level::warn,
                    std::pmr::string{std::error_code{std::abs(result), std::generic_category()}.message(),
                                     internal::getSyncMemoryResource()}
                });

                try {
                    context.getBufferRing().expandBuffer();
                } catch (internal::Exception &exception) { logger::write(std::move(exception.getLog())); }

                isRestart = true;

                break;
            }

            resumeFlags = asyncWaiter.getResumeFlags();

            std::span<const std::byte> data;
            if ((resumeFlags & IORING_CQE_F_BUFFER) != 0) {
                const std::uint32_t bufferId{resumeFlags >> IORING_CQE_BUFFER_SHIFT};
                data = context.getBufferRing().readData(bufferId, result);

                if ((resumeFlags & IORING_CQE_F_BUF_MORE) == 0) context.getBufferRing().markBufferUsed(bufferId);
            }

            action(result, data);
        } while ((resumeFlags & IORING_CQE_F_MORE) != 0);
    } while (isRestart);
}

auto coContext::write(const std::int32_t fileDescriptor, const std::span<const std::byte> buffer,
                      const std::uint64_t offset) -> internal::AsyncWaiter {
    return internal::AsyncWaiter{internal::Submission::write(context.getSubmission(), fileDescriptor, buffer, offset)};
}

auto coContext::write(const std::int32_t fileDescriptor, const std::span<const iovec> buffer,
                      const std::uint64_t offset) -> internal::AsyncWaiter {
    return internal::AsyncWaiter{internal::Submission::write(context.getSubmission(), fileDescriptor, buffer, offset)};
}

auto coContext::write(const std::int32_t fileDescriptor, const std::span<const iovec> buffer,
                      const std::uint64_t offset, const std::int32_t flags) -> internal::AsyncWaiter {
    return internal::AsyncWaiter{
        internal::Submission::write(context.getSubmission(), fileDescriptor, buffer, offset, flags)};
}

auto coContext::syncFile(const std::int32_t fileDescriptor, const bool isSyncMetadata) -> internal::AsyncWaiter {
    return internal::AsyncWaiter{internal::Submission::syncFile(context.getSubmission(), fileDescriptor,
                                                                isSyncMetadata ? 0 : IORING_FSYNC_DATASYNC)};
}

auto coContext::syncFile(const std::int32_t fileDescriptor, const std::uint64_t offset, const std::uint32_t length,
                         const std::int32_t flags) -> internal::AsyncWaiter {
    return internal::AsyncWaiter{
        internal::Submission::syncFile(context.getSubmission(), fileDescriptor, offset, length, flags)};
}

auto coContext::adviseFile(const std::int32_t fileDescriptor, const std::uint64_t offset, const off_t length,
                           const std::int32_t advice) -> internal::AsyncWaiter {
    return internal::AsyncWaiter{
        internal::Submission::adviseFile(context.getSubmission(), fileDescriptor, offset, length, advice)};
}

auto coContext::truncate(const std::int32_t fileDescriptor, const loff_t length) -> internal::AsyncWaiter {
    return internal::AsyncWaiter{internal::Submission::truncate(context.getSubmission(), fileDescriptor, length)};
}

auto coContext::allocateFile(const std::int32_t fileDescriptor, const std::int32_t mode, const std::uint64_t offset,
                             const std::uint64_t length) -> internal::AsyncWaiter {
    return internal::AsyncWaiter{
        internal::Submission::allocateFile(context.getSubmission(), fileDescriptor, mode, offset, length)};
}

auto coContext::status(const std::int32_t directoryFileDescriptor, const std::filesystem::path &path,
                       const std::int32_t flags, const std::uint32_t mask, struct statx &buffer)
    -> internal::AsyncWaiter {
    return internal::AsyncWaiter{
        internal::Submission::status(context.getSubmission(), directoryFileDescriptor, path, flags, mask, buffer)};
}

auto coContext::getExtendedAttribute(const std::filesystem::path &path, const std::string_view name,
                                     const std::span<char> value) -> internal::AsyncWaiter {
    return internal::AsyncWaiter{
        internal::Submission::getExtendedAttribute(context.getSubmission(), path, name, value)};
}

auto coContext::getExtendedAttribute(const std::int32_t fileDescriptor, const std::string_view name,
                                     const std::span<char> value) -> internal::AsyncWaiter {
    return internal::AsyncWaiter{
        internal::Submission::getExtendedAttribute(context.getSubmission(), fileDescriptor, name, value)};
}

auto coContext::setExtendedAttribute(const std::filesystem::path &path, const std::string_view name,
                                     const std::span<char> value, const std::int32_t flags) -> internal::AsyncWaiter {
    return internal::AsyncWaiter{
        internal::Submission::setExtendedAttribute(context.getSubmission(), path, name, value, flags)};
}

auto coContext::setExtendedAttribute(const std::int32_t fileDescriptor, const std::string_view name,
                                     const std::span<char> value, const std::int32_t flags) -> internal::AsyncWaiter {
    return internal::AsyncWaiter{
        internal::Submission::setExtendedAttribute(context.getSubmission(), fileDescriptor, name, value, flags)};
}

auto coContext::makeDirectory(const std::filesystem::path &path, const mode_t mode) -> internal::AsyncWaiter {
    return internal::AsyncWaiter{internal::Submission::makeDirectory(context.getSubmission(), path, mode)};
}

auto coContext::makeDirectory(const std::int32_t directoryFileDescriptor, const std::filesystem::path &path,
                              const mode_t mode) -> internal::AsyncWaiter {
    return internal::AsyncWaiter{
        internal::Submission::makeDirectory(context.getSubmission(), directoryFileDescriptor, path, mode)};
}

auto coContext::rename(const std::filesystem::path &oldPath, const std::filesystem::path &newPath)
    -> internal::AsyncWaiter {
    return internal::AsyncWaiter{internal::Submission::rename(context.getSubmission(), oldPath, newPath)};
}

auto coContext::rename(const std::int32_t oldDirectoryFileDescriptor, const std::filesystem::path &oldPath,
                       const std::int32_t newDirectoryFileDescriptor, const std::filesystem::path &newPath,
                       const std::uint32_t flags) -> internal::AsyncWaiter {
    return internal::AsyncWaiter{internal::Submission::rename(context.getSubmission(), oldDirectoryFileDescriptor,
                                                              oldPath, newDirectoryFileDescriptor, newPath, flags)};
}

auto coContext::link(const std::filesystem::path &oldPath, const std::filesystem::path &newPath)
    -> internal::AsyncWaiter {
    return internal::AsyncWaiter{internal::Submission::link(context.getSubmission(), oldPath, newPath, 0)};
}

auto coContext::link(const std::int32_t oldDirectoryFileDescriptor, const std::filesystem::path &oldPath,
                     const std::int32_t newDirectoryFileDescriptor, const std::filesystem::path &newPath,
                     const std::int32_t flags) -> internal::AsyncWaiter {
    return internal::AsyncWaiter{internal::Submission::link(context.getSubmission(), oldDirectoryFileDescriptor,
                                                            oldPath, newDirectoryFileDescriptor, newPath, flags)};
}

auto coContext::symbolicLink(const std::string_view target, const std::filesystem::path &linkPath)
    -> internal::AsyncWaiter {
    return internal::AsyncWaiter{internal::Submission::symbolicLink(context.getSubmission(), target, linkPath)};
}

auto coContext::symbolicLink(const std::string_view target, const std::int32_t newDirectoryFileDescriptor,
                             const std::filesystem::path &linkPath) -> internal::AsyncWaiter {
    return internal::AsyncWaiter{
        internal::Submission::symbolicLink(context.getSubmission(), target, newDirectoryFileDescriptor, linkPath)};
}

auto coContext::unlink(const std::filesystem::path &path) -> internal::AsyncWaiter {
    return internal::AsyncWaiter{internal::Submission::unlink(context.getSubmission(), path, 0)};
}

auto coContext::unlink(const std::int32_t directoryFileDescriptor, const std::filesystem::path &path,
                       const std::int32_t flags) -> internal::AsyncWaiter {
    return internal::AsyncWaiter{
        internal::Submission::unlink(context.getSubmission(), directoryFileDescriptor, path, flags)};
}

auto coContext::adviseMemory(const std::span<std::byte> buffer, const std::int32_t advice) -> internal::AsyncWaiter {
    return internal::AsyncWaiter{internal::Submission::adviseMemory(context.getSubmission(), buffer, advice)};
}

auto coContext::wait(const idtype_t idType, const id_t id, siginfo_t *const signalInformation,
                     const std::int32_t options) -> internal::AsyncWaiter {
    return internal::AsyncWaiter{
        internal::Submission::wait(context.getSubmission(), idType, id, signalInformation, options, 0)};
}

auto coContext::waitFutex(std::uint32_t &futex, const std::uint64_t value, const std::uint64_t mask,
                          const std::uint32_t flags) -> internal::AsyncWaiter {
    return internal::AsyncWaiter{
        internal::Submission::waitFutex(context.getSubmission(), futex, value, mask, flags, 0)};
}

auto coContext::waitFutex(const std::span<futex_waitv> vectorizedFutexs) -> internal::AsyncWaiter {
    return internal::AsyncWaiter{internal::Submission::waitFutex(context.getSubmission(), vectorizedFutexs, 0)};
}

auto coContext::wakeFutex(std::uint32_t &futex, const std::uint64_t value, const std::uint64_t mask,
                          const std::uint32_t flags) -> internal::AsyncWaiter {
    return internal::AsyncWaiter{
        internal::Submission::waitFutex(context.getSubmission(), futex, value, mask, flags, 0)};
}

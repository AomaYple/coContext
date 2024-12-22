#include "coContext/ring/SubmissionQueueEntry.hpp"

#include <liburing.h>

coContext::SubmissionQueueEntry::SubmissionQueueEntry(io_uring_sqe *const handle) noexcept : handle{handle} {}

auto coContext::SubmissionQueueEntry::get() const noexcept -> io_uring_sqe * { return this->handle; }

auto coContext::SubmissionQueueEntry::addFlags(const std::uint32_t flags) const noexcept -> void {
    io_uring_sqe_set_flags(this->handle, this->handle->flags | flags);
}

auto coContext::SubmissionQueueEntry::addIoPriority(const std::uint16_t ioPriority) const noexcept -> void {
    this->handle->ioprio |= ioPriority;
}

auto coContext::SubmissionQueueEntry::setUserData(const std::uint64_t userData) const noexcept -> void {
    io_uring_sqe_set_data64(this->handle, userData);
}

auto coContext::SubmissionQueueEntry::linkTimeout(__kernel_timespec &timeSpecification,
                                                  const std::uint32_t flags) const noexcept -> void {
    io_uring_prep_link_timeout(this->handle, std::addressof(timeSpecification), flags);
}

auto coContext::SubmissionQueueEntry::cancel(const std::uint64_t userData, const std::int32_t flags) const noexcept
    -> void {
    io_uring_prep_cancel64(this->handle, userData, flags);
}

auto coContext::SubmissionQueueEntry::cancel(const std::int32_t fileDescriptor,
                                             const std::uint32_t flags) const noexcept -> void {
    io_uring_prep_cancel_fd(this->handle, fileDescriptor, flags);
}

auto coContext::SubmissionQueueEntry::timeout(__kernel_timespec &timeSpecification, const std::uint32_t count,
                                              const std::uint32_t flags) const noexcept -> void {
    io_uring_prep_timeout(this->handle, std::addressof(timeSpecification), count, flags);
}

auto coContext::SubmissionQueueEntry::updateTimeout(const std::uint64_t userData, __kernel_timespec &timeSpecification,
                                                    const std::uint32_t flags) const noexcept -> void {
    io_uring_prep_timeout_update(this->handle, std::addressof(timeSpecification), userData, flags);
}

auto coContext::SubmissionQueueEntry::poll(const std::int32_t fileDescriptor, const std::uint32_t mask) const noexcept
    -> void {
    io_uring_prep_poll_add(this->handle, fileDescriptor, mask);
}

auto coContext::SubmissionQueueEntry::updatePoll(const std::uint64_t oldUserData, const std::uint64_t newUserData,
                                                 const std::uint32_t mask, const std::uint32_t flags) const noexcept
    -> void {
    io_uring_prep_poll_update(this->handle, oldUserData, newUserData, mask, flags);
}

auto coContext::SubmissionQueueEntry::multiplePoll(const std::int32_t fileDescriptor,
                                                   const std::uint32_t mask) const noexcept -> void {
    io_uring_prep_poll_multishot(this->handle, fileDescriptor, mask);
}

auto coContext::SubmissionQueueEntry::installDirectFileDescriptor(const std::int32_t directFileDescriptor,
                                                                  const std::uint32_t flags) const noexcept -> void {
    io_uring_prep_fixed_fd_install(this->handle, directFileDescriptor, flags);
}

auto coContext::SubmissionQueueEntry::close(const std::int32_t fileDescriptor) const noexcept -> void {
    io_uring_prep_close(this->handle, fileDescriptor);
}

auto coContext::SubmissionQueueEntry::closeDirect(const std::int32_t directFileDescriptor) const noexcept -> void {
    io_uring_prep_close_direct(this->handle, directFileDescriptor);
}

auto coContext::SubmissionQueueEntry::socket(const std::int32_t domain, const std::int32_t type,
                                             const std::int32_t protocol, const std::uint32_t flags) const noexcept
    -> void {
    io_uring_prep_socket(this->handle, domain, type, protocol, flags);
}

auto coContext::SubmissionQueueEntry::directSocket(const std::int32_t domain, const std::int32_t type,
                                                   const std::int32_t protocol,
                                                   const std::uint32_t flags) const noexcept -> void {
    io_uring_prep_socket_direct_alloc(this->handle, domain, type, protocol, flags);
}

auto coContext::SubmissionQueueEntry::socketCommand(const std::int32_t operation,
                                                    const std::int32_t socketFileDescriptor, const std::int32_t level,
                                                    const std::int32_t optionName, void *const optionValue,
                                                    const std::int32_t optionLength) const noexcept -> void {
    io_uring_prep_cmd_sock(this->handle, operation, socketFileDescriptor, level, optionName, optionValue, optionLength);
}

auto coContext::SubmissionQueueEntry::discardCommand(const std::int32_t fileDescriptor, const std::uint64_t offset,
                                                     const std::uint64_t length) const noexcept -> void {
    io_uring_prep_cmd_discard(this->handle, fileDescriptor, offset, length);
}

auto coContext::SubmissionQueueEntry::bind(const std::int32_t socketFileDescriptor, sockaddr &address,
                                           const socklen_t addressLength) const noexcept -> void {
    io_uring_prep_bind(this->handle, socketFileDescriptor, std::addressof(address), addressLength);
}

auto coContext::SubmissionQueueEntry::listen(const std::int32_t socketFileDescriptor,
                                             const std::int32_t backlog) const noexcept -> void {
    io_uring_prep_listen(this->handle, socketFileDescriptor, backlog);
}

auto coContext::SubmissionQueueEntry::accept(const std::int32_t socketFileDescriptor, sockaddr *const address,
                                             socklen_t *const addressLength, const std::int32_t flags) const noexcept
    -> void {
    io_uring_prep_accept(this->handle, socketFileDescriptor, address, addressLength, flags);
}

auto coContext::SubmissionQueueEntry::acceptDirect(const std::int32_t socketFileDescriptor, sockaddr *const address,
                                                   socklen_t *const addressLength, const std::int32_t flags,
                                                   const std::uint32_t fileDescriptorIndex) const noexcept -> void {
    io_uring_prep_accept_direct(this->handle, socketFileDescriptor, address, addressLength, flags, fileDescriptorIndex);
}

auto coContext::SubmissionQueueEntry::multipleAccept(const std::int32_t socketFileDescriptor, sockaddr *const address,
                                                     socklen_t *const addressLength,
                                                     const std::int32_t flags) const noexcept -> void {
    io_uring_prep_multishot_accept(this->handle, socketFileDescriptor, address, addressLength, flags);
}

auto coContext::SubmissionQueueEntry::multipleAcceptDirect(const std::int32_t socketFileDescriptor,
                                                           sockaddr *const address, socklen_t *const addressLength,
                                                           const std::int32_t flags) const noexcept -> void {
    io_uring_prep_multishot_accept_direct(this->handle, socketFileDescriptor, address, addressLength, flags);
}

auto coContext::SubmissionQueueEntry::connect(const std::int32_t socketFileDescriptor, const sockaddr &address,
                                              const socklen_t addressLength) const noexcept -> void {
    io_uring_prep_connect(this->handle, socketFileDescriptor, std::addressof(address), addressLength);
}

auto coContext::SubmissionQueueEntry::shutdown(const std::int32_t socketFileDescriptor,
                                               const std::int32_t how) const noexcept -> void {
    io_uring_prep_shutdown(this->handle, socketFileDescriptor, how);
}

auto coContext::SubmissionQueueEntry::receive(const std::int32_t socketFileDescriptor,
                                              const std::span<std::byte> buffer,
                                              const std::int32_t flags) const noexcept -> void {
    io_uring_prep_recv(this->handle, socketFileDescriptor, std::data(buffer), std::size(buffer), flags);
}

auto coContext::SubmissionQueueEntry::receive(const std::int32_t socketFileDescriptor, msghdr &message,
                                              const std::uint32_t flags) const noexcept -> void {
    io_uring_prep_recvmsg(this->handle, socketFileDescriptor, std::addressof(message), flags);
}

auto coContext::SubmissionQueueEntry::send(const std::int32_t socketFileDescriptor,
                                           const std::span<const std::byte> buffer,
                                           const std::int32_t flags) const noexcept -> void {
    io_uring_prep_send(this->handle, socketFileDescriptor, std::data(buffer), std::size(buffer), flags);
}

auto coContext::SubmissionQueueEntry::send(const std::int32_t socketFileDescriptor,
                                           const std::span<const std::byte> buffer, const std::int32_t flags,
                                           const sockaddr &destinationAddress,
                                           const socklen_t destinationAddressLength) const noexcept -> void {
    io_uring_prep_sendto(this->handle, socketFileDescriptor, std::data(buffer), std::size(buffer), flags,
                         std::addressof(destinationAddress), destinationAddressLength);
}

auto coContext::SubmissionQueueEntry::send(const std::int32_t socketFileDescriptor, const msghdr &message,
                                           const std::uint32_t flags) const noexcept -> void {
    io_uring_prep_sendmsg(this->handle, socketFileDescriptor, std::addressof(message), flags);
}

auto coContext::SubmissionQueueEntry::splice(const std::int32_t inFileDescriptor,
                                             const std::int64_t inFileDescriptorOffset,
                                             const std::int32_t outFileDescriptor,
                                             const std::int64_t outFileDescriptorOffset, const std::uint32_t length,
                                             const std::uint32_t flags) const noexcept -> void {
    io_uring_prep_splice(this->handle, inFileDescriptor, inFileDescriptorOffset, outFileDescriptor,
                         outFileDescriptorOffset, length, flags);
}

auto coContext::SubmissionQueueEntry::tee(const std::int32_t inFileDescriptor, const std::int32_t outFileDescriptor,
                                          const std::uint32_t length, const std::uint32_t flags) const noexcept
    -> void {
    io_uring_prep_tee(this->handle, inFileDescriptor, outFileDescriptor, length, flags);
}

auto coContext::SubmissionQueueEntry::open(const std::string_view path, const std::int32_t flags,
                                           const mode_t mode) const noexcept -> void {
    io_uring_prep_open(this->handle, std::data(path), flags, mode);
}

auto coContext::SubmissionQueueEntry::open(const std::int32_t directoryFileDescriptor, const std::string_view path,
                                           const std::int32_t flags, const mode_t mode) const noexcept -> void {
    io_uring_prep_openat(this->handle, directoryFileDescriptor, std::data(path), flags, mode);
}

auto coContext::SubmissionQueueEntry::open(const std::int32_t directoryFileDescriptor, const std::string_view path,
                                           open_how &openHow) const noexcept -> void {
    io_uring_prep_openat2(this->handle, directoryFileDescriptor, std::data(path), std::addressof(openHow));
}

auto coContext::SubmissionQueueEntry::openDirect(const std::string_view path, const std::int32_t flags,
                                                 const mode_t mode,
                                                 const std::uint32_t fileDescriptorIndex) const noexcept -> void {
    io_uring_prep_open_direct(this->handle, std::data(path), flags, mode, fileDescriptorIndex);
}

auto coContext::SubmissionQueueEntry::openDirect(const std::int32_t directoryFileDescriptor,
                                                 const std::string_view path, const std::int32_t flags,
                                                 const mode_t mode,
                                                 const std::uint32_t fileDescriptorIndex) const noexcept -> void {
    io_uring_prep_openat_direct(this->handle, directoryFileDescriptor, std::data(path), flags, mode,
                                fileDescriptorIndex);
}

auto coContext::SubmissionQueueEntry::openDirect(const std::int32_t directoryFileDescriptor,
                                                 const std::string_view path, open_how &openHow,
                                                 const std::uint32_t fileDescriptorIndex) const noexcept -> void {
    io_uring_prep_openat2_direct(this->handle, directoryFileDescriptor, std::data(path), std::addressof(openHow),
                                 fileDescriptorIndex);
}

auto coContext::SubmissionQueueEntry::read(const std::int32_t fileDescriptor, const std::span<std::byte> buffer,
                                           const std::uint64_t offset) const noexcept -> void {
    io_uring_prep_read(this->handle, fileDescriptor, std::data(buffer), std::size(buffer), offset);
}

auto coContext::SubmissionQueueEntry::read(const std::int32_t fileDescriptor, const std::span<const iovec> buffer,
                                           const std::uint64_t offset) const noexcept -> void {
    io_uring_prep_readv(this->handle, fileDescriptor, std::data(buffer), std::size(buffer), offset);
}

auto coContext::SubmissionQueueEntry::read(const std::int32_t fileDescriptor, const std::span<const iovec> buffer,
                                           const std::uint64_t offset, const std::int32_t flags) const noexcept
    -> void {
    io_uring_prep_readv2(this->handle, fileDescriptor, std::data(buffer), std::size(buffer), offset, flags);
}

auto coContext::SubmissionQueueEntry::write(const std::int32_t fileDescriptor, const std::span<const std::byte> buffer,
                                            const std::uint64_t offset) const noexcept -> void {
    io_uring_prep_write(this->handle, fileDescriptor, std::data(buffer), std::size(buffer), offset);
}

auto coContext::SubmissionQueueEntry::write(const std::int32_t fileDescriptor, const std::span<const iovec> buffer,
                                            const std::uint64_t offset) const noexcept -> void {
    io_uring_prep_writev(this->handle, fileDescriptor, std::data(buffer), std::size(buffer), offset);
}

auto coContext::SubmissionQueueEntry::write(const std::int32_t fileDescriptor, const std::span<const iovec> buffer,
                                            const std::uint64_t offset, const std::int32_t flags) const noexcept
    -> void {
    io_uring_prep_writev2(this->handle, fileDescriptor, std::data(buffer), std::size(buffer), offset, flags);
}

auto coContext::SubmissionQueueEntry::syncFile(const std::int32_t fileDescriptor,
                                               const std::uint32_t flags) const noexcept -> void {
    io_uring_prep_fsync(this->handle, fileDescriptor, flags);
}

auto coContext::SubmissionQueueEntry::syncFile(const std::int32_t fileDescriptor, const std::uint64_t offset,
                                               const std::uint32_t length, const std::int32_t flags) const noexcept
    -> void {
    io_uring_prep_sync_file_range(this->handle, fileDescriptor, length, offset, flags);
}

auto coContext::SubmissionQueueEntry::adviseFile(const std::int32_t fileDescriptor, const std::uint64_t offset,
                                                 const off_t length, const std::int32_t advice) const noexcept -> void {
    io_uring_prep_fadvise64(this->handle, fileDescriptor, offset, length, advice);
}

auto coContext::SubmissionQueueEntry::truncate(const std::int32_t fileDescriptor, const loff_t length) const noexcept
    -> void {
    io_uring_prep_ftruncate(this->handle, fileDescriptor, length);
}

auto coContext::SubmissionQueueEntry::allocateFile(const std::int32_t fileDescriptor, const std::int32_t mode,
                                                   const std::uint64_t offset,
                                                   const std::uint64_t length) const noexcept -> void {
    io_uring_prep_fallocate(this->handle, fileDescriptor, mode, offset, length);
}

auto coContext::SubmissionQueueEntry::getFileStatus(const std::int32_t directoryFileDescriptor,
                                                    const std::string_view path, const std::int32_t flags,
                                                    const std::uint32_t mask, struct statx &buffer) const noexcept
    -> void {
    io_uring_prep_statx(this->handle, directoryFileDescriptor, std::data(path), flags, mask, std::addressof(buffer));
}

auto coContext::SubmissionQueueEntry::getExtendedAttribute(const std::string_view path, const std::string_view name,
                                                           const std::span<char> value) const noexcept -> void {
    io_uring_prep_getxattr(this->handle, std::data(name), std::data(value), std::data(path), std::size(value));
}

auto coContext::SubmissionQueueEntry::getExtendedAttribute(const std::int32_t fileDescriptor,
                                                           const std::string_view name,
                                                           const std::span<char> value) const noexcept -> void {
    io_uring_prep_fgetxattr(this->handle, fileDescriptor, std::data(name), std::data(value), std::size(value));
}

auto coContext::SubmissionQueueEntry::setExtendedAttribute(const std::string_view path, const std::string_view name,
                                                           const std::span<char> value,
                                                           const std::int32_t flags) const noexcept -> void {
    io_uring_prep_setxattr(this->handle, std::data(name), std::data(value), std::data(path), flags, std::size(value));
}

auto coContext::SubmissionQueueEntry::setExtendedAttribute(const std::int32_t fileDescriptor,
                                                           const std::string_view name, const std::span<char> value,
                                                           const std::int32_t flags) const noexcept -> void {
    io_uring_prep_fsetxattr(this->handle, fileDescriptor, std::data(name), std::data(value), flags, std::size(value));
}

auto coContext::SubmissionQueueEntry::makeDirectory(const std::string_view path, const mode_t mode) const noexcept
    -> void {
    io_uring_prep_mkdir(this->handle, std::data(path), mode);
}

auto coContext::SubmissionQueueEntry::makeDirectory(const std::int32_t directoryFileDescriptor,
                                                    const std::string_view path, const mode_t mode) const noexcept
    -> void {
    io_uring_prep_mkdirat(this->handle, directoryFileDescriptor, std::data(path), mode);
}

auto coContext::SubmissionQueueEntry::rename(const std::string_view oldPath,
                                             const std::string_view newPath) const noexcept -> void {
    io_uring_prep_rename(this->handle, std::data(oldPath), std::data(newPath));
}

auto coContext::SubmissionQueueEntry::rename(const std::int32_t oldDirectoryFileDescriptor,
                                             const std::string_view oldPath,
                                             const std::int32_t newDirectoryFileDescriptor,
                                             const std::string_view newPath, const std::uint32_t flags) const noexcept
    -> void {
    io_uring_prep_renameat(this->handle, oldDirectoryFileDescriptor, std::data(oldPath), newDirectoryFileDescriptor,
                           std::data(newPath), flags);
}

auto coContext::SubmissionQueueEntry::link(const std::string_view oldPath, const std::string_view newPath,
                                           const std::int32_t flags) const noexcept -> void {
    io_uring_prep_link(this->handle, std::data(oldPath), std::data(newPath), flags);
}

auto coContext::SubmissionQueueEntry::link(const std::int32_t oldDirectoryFileDescriptor,
                                           const std::string_view oldPath,
                                           const std::int32_t newDirectoryFileDescriptor,
                                           const std::string_view newPath, const std::int32_t flags) const noexcept
    -> void {
    io_uring_prep_linkat(this->handle, oldDirectoryFileDescriptor, std::data(oldPath), newDirectoryFileDescriptor,
                         std::data(newPath), flags);
}

auto coContext::SubmissionQueueEntry::symbolicLink(const std::string_view target,
                                                   const std::string_view linkPath) const noexcept -> void {
    io_uring_prep_symlink(this->handle, std::data(target), std::data(linkPath));
}

auto coContext::SubmissionQueueEntry::symbolicLink(const std::string_view target,
                                                   const std::int32_t newDirectoryFileDescriptor,
                                                   const std::string_view linkPath) const noexcept -> void {
    io_uring_prep_symlinkat(this->handle, std::data(target), newDirectoryFileDescriptor, std::data(linkPath));
}

auto coContext::SubmissionQueueEntry::unlink(const std::string_view path, const std::int32_t flags) const noexcept
    -> void {
    io_uring_prep_unlink(this->handle, std::data(path), flags);
}

auto coContext::SubmissionQueueEntry::unlink(const std::int32_t directoryFileDescriptor, const std::string_view path,
                                             const std::int32_t flags) const noexcept -> void {
    io_uring_prep_unlinkat(this->handle, directoryFileDescriptor, std::data(path), flags);
}

auto coContext::SubmissionQueueEntry::adviseMemory(const std::span<std::byte> buffer,
                                                   const std::int32_t advice) const noexcept -> void {
    io_uring_prep_madvise64(this->handle, std::data(buffer), static_cast<off_t>(std::size(buffer)), advice);
}

auto coContext::SubmissionQueueEntry::wait(const idtype_t idType, const id_t id, siginfo_t *const signalInformation,
                                           const std::int32_t options, const std::uint32_t flags) const noexcept
    -> void {
    io_uring_prep_waitid(this->handle, idType, id, signalInformation, options, flags);
}

auto coContext::SubmissionQueueEntry::waitFutex(std::uint32_t &futex, const std::uint64_t value,
                                                const std::uint64_t mask, const std::uint32_t futexFlags,
                                                const std::uint32_t flags) const noexcept -> void {
    io_uring_prep_futex_wait(this->handle, std::addressof(futex), value, mask, futexFlags, flags);
}

auto coContext::SubmissionQueueEntry::wakeFutex(std::uint32_t &futex, const std::uint64_t value,
                                                const std::uint64_t mask, const std::uint32_t futexFlags,
                                                const std::uint32_t flags) const noexcept -> void {
    io_uring_prep_futex_wake(this->handle, std::addressof(futex), value, mask, futexFlags, flags);
}

auto coContext::operator==(const SubmissionQueueEntry lhs, const SubmissionQueueEntry rhs) noexcept -> bool {
    return lhs.get() == rhs.get();
}

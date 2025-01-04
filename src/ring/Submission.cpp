#include "coContext/ring/Submission.hpp"

coContext::internal::Submission::Submission(io_uring_sqe *const handle) noexcept : handle{handle} {}

auto coContext::internal::Submission::get() const noexcept -> io_uring_sqe * { return this->handle; }

auto coContext::internal::Submission::addFlags(const std::uint32_t flags) const noexcept -> void {
    io_uring_sqe_set_flags(this->handle, this->handle->flags | flags);
}

auto coContext::internal::Submission::addIoPriority(const std::uint16_t ioPriority) const noexcept -> void {
    this->handle->ioprio |= ioPriority;
}

auto coContext::internal::Submission::setUserData(const std::uint64_t userData) const noexcept -> void {
    io_uring_sqe_set_data64(this->handle, userData);
}

auto coContext::internal::Submission::setBufferGroup(const std::uint16_t bufferGroup) const noexcept -> void {
    this->handle->buf_group = bufferGroup;
}

auto coContext::internal::Submission::updateFileDescriptors(const std::span<std::int32_t> fileDescriptors,
                                                            const std::int32_t offset) const noexcept -> void {
    io_uring_prep_files_update(this->handle, std::data(fileDescriptors), std::size(fileDescriptors), offset);
}

auto coContext::internal::Submission::installDirect(const std::int32_t directFileDescriptor,
                                                    const std::uint32_t flags) const noexcept -> void {
    io_uring_prep_fixed_fd_install(this->handle, directFileDescriptor, flags);
}

auto coContext::internal::Submission::linkTimeout(__kernel_timespec &timeSpecification,
                                                  const std::uint32_t flags) const noexcept -> void {
    io_uring_prep_link_timeout(this->handle, std::addressof(timeSpecification), flags);
}

auto coContext::internal::Submission::cancel(const std::uint64_t userData, const std::int32_t flags) const noexcept
    -> void {
    io_uring_prep_cancel64(this->handle, userData, flags);
}

auto coContext::internal::Submission::cancel(const std::int32_t fileDescriptor,
                                             const std::uint32_t flags) const noexcept -> void {
    io_uring_prep_cancel_fd(this->handle, fileDescriptor, flags);
}

auto coContext::internal::Submission::timeout(__kernel_timespec &timeSpecification, const std::uint32_t count,
                                              const std::uint32_t flags) const noexcept -> void {
    io_uring_prep_timeout(this->handle, std::addressof(timeSpecification), count, flags);
}

auto coContext::internal::Submission::updateTimeout(const std::uint64_t userData, __kernel_timespec &timeSpecification,
                                                    const std::uint32_t flags) const noexcept -> void {
    io_uring_prep_timeout_update(this->handle, std::addressof(timeSpecification), userData, flags);
}

auto coContext::internal::Submission::poll(const std::int32_t fileDescriptor, const std::uint32_t mask) const noexcept
    -> void {
    io_uring_prep_poll_add(this->handle, fileDescriptor, mask);
}

auto coContext::internal::Submission::updatePoll(const std::uint64_t oldUserData, const std::uint64_t newUserData,
                                                 const std::uint32_t mask, const std::uint32_t flags) const noexcept
    -> void {
    io_uring_prep_poll_update(this->handle, oldUserData, newUserData, mask, flags);
}

auto coContext::internal::Submission::multiplePoll(const std::int32_t fileDescriptor,
                                                   const std::uint32_t mask) const noexcept -> void {
    io_uring_prep_poll_multishot(this->handle, fileDescriptor, mask);
}

auto coContext::internal::Submission::close(const std::int32_t fileDescriptor) const noexcept -> void {
    io_uring_prep_close(this->handle, fileDescriptor);
}

auto coContext::internal::Submission::closeDirect(const std::int32_t directFileDescriptor) const noexcept -> void {
    io_uring_prep_close_direct(this->handle, directFileDescriptor);
}

auto coContext::internal::Submission::socket(const std::int32_t domain, const std::int32_t type,
                                             const std::int32_t protocol, const std::uint32_t flags) const noexcept
    -> void {
    io_uring_prep_socket(this->handle, domain, type, protocol, flags);
}

auto coContext::internal::Submission::directSocket(const std::int32_t domain, const std::int32_t type,
                                                   const std::int32_t protocol,
                                                   const std::uint32_t flags) const noexcept -> void {
    io_uring_prep_socket_direct_alloc(this->handle, domain, type, protocol, flags);
}

auto coContext::internal::Submission::socketCommand(const std::int32_t operation,
                                                    const std::int32_t socketFileDescriptor, const std::int32_t level,
                                                    const std::int32_t optionName,
                                                    const std::span<std::byte> option) const noexcept -> void {
    io_uring_prep_cmd_sock(this->handle, operation, socketFileDescriptor, level, optionName, std::data(option),
                           static_cast<std::int32_t>(std::size(option)));
}

auto coContext::internal::Submission::discardCommand(const std::int32_t fileDescriptor, const std::uint64_t offset,
                                                     const std::uint64_t length) const noexcept -> void {
    io_uring_prep_cmd_discard(this->handle, fileDescriptor, offset, length);
}

auto coContext::internal::Submission::bind(const std::int32_t socketFileDescriptor, sockaddr &address,
                                           const socklen_t addressLength) const noexcept -> void {
    io_uring_prep_bind(this->handle, socketFileDescriptor, std::addressof(address), addressLength);
}

auto coContext::internal::Submission::listen(const std::int32_t socketFileDescriptor,
                                             const std::int32_t backlog) const noexcept -> void {
    io_uring_prep_listen(this->handle, socketFileDescriptor, backlog);
}

auto coContext::internal::Submission::accept(const std::int32_t socketFileDescriptor, sockaddr *const address,
                                             socklen_t *const addressLength, const std::int32_t flags) const noexcept
    -> void {
    io_uring_prep_accept(this->handle, socketFileDescriptor, address, addressLength, flags);
}

auto coContext::internal::Submission::acceptDirect(const std::int32_t socketFileDescriptor, sockaddr *const address,
                                                   socklen_t *const addressLength, const std::int32_t flags,
                                                   const std::uint32_t fileDescriptorIndex) const noexcept -> void {
    io_uring_prep_accept_direct(this->handle, socketFileDescriptor, address, addressLength, flags, fileDescriptorIndex);
}

auto coContext::internal::Submission::multipleAccept(const std::int32_t socketFileDescriptor, sockaddr *const address,
                                                     socklen_t *const addressLength,
                                                     const std::int32_t flags) const noexcept -> void {
    io_uring_prep_multishot_accept(this->handle, socketFileDescriptor, address, addressLength, flags);
}

auto coContext::internal::Submission::multipleAcceptDirect(const std::int32_t socketFileDescriptor,
                                                           sockaddr *const address, socklen_t *const addressLength,
                                                           const std::int32_t flags) const noexcept -> void {
    io_uring_prep_multishot_accept_direct(this->handle, socketFileDescriptor, address, addressLength, flags);
}

auto coContext::internal::Submission::connect(const std::int32_t socketFileDescriptor, const sockaddr &address,
                                              const socklen_t addressLength) const noexcept -> void {
    io_uring_prep_connect(this->handle, socketFileDescriptor, std::addressof(address), addressLength);
}

auto coContext::internal::Submission::shutdown(const std::int32_t socketFileDescriptor,
                                               const std::int32_t how) const noexcept -> void {
    io_uring_prep_shutdown(this->handle, socketFileDescriptor, how);
}

auto coContext::internal::Submission::receive(const std::int32_t socketFileDescriptor,
                                              const std::span<std::byte> buffer,
                                              const std::int32_t flags) const noexcept -> void {
    io_uring_prep_recv(this->handle, socketFileDescriptor, std::data(buffer), std::size(buffer), flags);
}

auto coContext::internal::Submission::receive(const std::int32_t socketFileDescriptor, msghdr &message,
                                              const std::uint32_t flags) const noexcept -> void {
    io_uring_prep_recvmsg(this->handle, socketFileDescriptor, std::addressof(message), flags);
}

auto coContext::internal::Submission::multipleReceive(const std::int32_t socketFileDescriptor,
                                                      const std::span<std::byte> buffer,
                                                      const std::int32_t flags) const noexcept -> void {
    io_uring_prep_recv_multishot(this->handle, socketFileDescriptor, std::data(buffer), std::size(buffer), flags);
}

auto coContext::internal::Submission::send(const std::int32_t socketFileDescriptor,
                                           const std::span<const std::byte> buffer,
                                           const std::int32_t flags) const noexcept -> void {
    io_uring_prep_send(this->handle, socketFileDescriptor, std::data(buffer), std::size(buffer), flags);
}

auto coContext::internal::Submission::send(const std::int32_t socketFileDescriptor,
                                           const std::span<const std::byte> buffer, const std::int32_t flags,
                                           const sockaddr &destinationAddress,
                                           const socklen_t destinationAddressLength) const noexcept -> void {
    io_uring_prep_sendto(this->handle, socketFileDescriptor, std::data(buffer), std::size(buffer), flags,
                         std::addressof(destinationAddress), destinationAddressLength);
}

auto coContext::internal::Submission::send(const std::int32_t socketFileDescriptor, const msghdr &message,
                                           const std::uint32_t flags) const noexcept -> void {
    io_uring_prep_sendmsg(this->handle, socketFileDescriptor, std::addressof(message), flags);
}

auto coContext::internal::Submission::zeroCopySend(const std::int32_t socketFileDescriptor,
                                                   const std::span<const std::byte> buffer, const std::int32_t flags,
                                                   const std::uint32_t zeroCopyFlags) const noexcept -> void {
    io_uring_prep_send_zc(this->handle, socketFileDescriptor, std::data(buffer), std::size(buffer), flags,
                          zeroCopyFlags);
}

auto coContext::internal::Submission::zeroCopySend(const std::int32_t socketFileDescriptor, const msghdr &message,
                                                   const std::uint32_t flags) const noexcept -> void {
    io_uring_prep_sendmsg_zc(this->handle, socketFileDescriptor, std::addressof(message), flags);
}

auto coContext::internal::Submission::splice(const std::int32_t inFileDescriptor,
                                             const std::int64_t inFileDescriptorOffset,
                                             const std::int32_t outFileDescriptor,
                                             const std::int64_t outFileDescriptorOffset, const std::uint32_t length,
                                             const std::uint32_t flags) const noexcept -> void {
    io_uring_prep_splice(this->handle, inFileDescriptor, inFileDescriptorOffset, outFileDescriptor,
                         outFileDescriptorOffset, length, flags);
}

auto coContext::internal::Submission::tee(const std::int32_t inFileDescriptor, const std::int32_t outFileDescriptor,
                                          const std::uint32_t length, const std::uint32_t flags) const noexcept
    -> void {
    io_uring_prep_tee(this->handle, inFileDescriptor, outFileDescriptor, length, flags);
}

auto coContext::internal::Submission::open(const std::string_view path, const std::int32_t flags,
                                           const mode_t mode) const noexcept -> void {
    io_uring_prep_open(this->handle, std::data(path), flags, mode);
}

auto coContext::internal::Submission::open(const std::int32_t directoryFileDescriptor, const std::string_view path,
                                           const std::int32_t flags, const mode_t mode) const noexcept -> void {
    io_uring_prep_openat(this->handle, directoryFileDescriptor, std::data(path), flags, mode);
}

auto coContext::internal::Submission::open(const std::int32_t directoryFileDescriptor, const std::string_view path,
                                           open_how &openHow) const noexcept -> void {
    io_uring_prep_openat2(this->handle, directoryFileDescriptor, std::data(path), std::addressof(openHow));
}

auto coContext::internal::Submission::openDirect(const std::string_view path, const std::int32_t flags,
                                                 const mode_t mode,
                                                 const std::uint32_t fileDescriptorIndex) const noexcept -> void {
    io_uring_prep_open_direct(this->handle, std::data(path), flags, mode, fileDescriptorIndex);
}

auto coContext::internal::Submission::openDirect(const std::int32_t directoryFileDescriptor,
                                                 const std::string_view path, const std::int32_t flags,
                                                 const mode_t mode,
                                                 const std::uint32_t fileDescriptorIndex) const noexcept -> void {
    io_uring_prep_openat_direct(this->handle, directoryFileDescriptor, std::data(path), flags, mode,
                                fileDescriptorIndex);
}

auto coContext::internal::Submission::openDirect(const std::int32_t directoryFileDescriptor,
                                                 const std::string_view path, open_how &openHow,
                                                 const std::uint32_t fileDescriptorIndex) const noexcept -> void {
    io_uring_prep_openat2_direct(this->handle, directoryFileDescriptor, std::data(path), std::addressof(openHow),
                                 fileDescriptorIndex);
}

auto coContext::internal::Submission::read(const std::int32_t fileDescriptor, const std::span<std::byte> buffer,
                                           const std::uint64_t offset) const noexcept -> void {
    io_uring_prep_read(this->handle, fileDescriptor, std::data(buffer), std::size(buffer), offset);
}

auto coContext::internal::Submission::read(const std::int32_t fileDescriptor, const std::span<const iovec> buffer,
                                           const std::uint64_t offset) const noexcept -> void {
    io_uring_prep_readv(this->handle, fileDescriptor, std::data(buffer), std::size(buffer), offset);
}

auto coContext::internal::Submission::read(const std::int32_t fileDescriptor, const std::span<const iovec> buffer,
                                           const std::uint64_t offset, const std::int32_t flags) const noexcept
    -> void {
    io_uring_prep_readv2(this->handle, fileDescriptor, std::data(buffer), std::size(buffer), offset, flags);
}

auto coContext::internal::Submission::multipleRead(const std::int32_t fileDescriptor, const std::uint32_t length,
                                                   const std::uint64_t offset,
                                                   const std::int32_t bufferGroup) const noexcept -> void {
    io_uring_prep_read_multishot(this->handle, fileDescriptor, length, offset, bufferGroup);
}

auto coContext::internal::Submission::write(const std::int32_t fileDescriptor, const std::span<const std::byte> buffer,
                                            const std::uint64_t offset) const noexcept -> void {
    io_uring_prep_write(this->handle, fileDescriptor, std::data(buffer), std::size(buffer), offset);
}

auto coContext::internal::Submission::write(const std::int32_t fileDescriptor, const std::span<const iovec> buffer,
                                            const std::uint64_t offset) const noexcept -> void {
    io_uring_prep_writev(this->handle, fileDescriptor, std::data(buffer), std::size(buffer), offset);
}

auto coContext::internal::Submission::write(const std::int32_t fileDescriptor, const std::span<const iovec> buffer,
                                            const std::uint64_t offset, const std::int32_t flags) const noexcept
    -> void {
    io_uring_prep_writev2(this->handle, fileDescriptor, std::data(buffer), std::size(buffer), offset, flags);
}

auto coContext::internal::Submission::syncFile(const std::int32_t fileDescriptor,
                                               const std::uint32_t flags) const noexcept -> void {
    io_uring_prep_fsync(this->handle, fileDescriptor, flags);
}

auto coContext::internal::Submission::syncFile(const std::int32_t fileDescriptor, const std::uint64_t offset,
                                               const std::uint32_t length, const std::int32_t flags) const noexcept
    -> void {
    io_uring_prep_sync_file_range(this->handle, fileDescriptor, length, offset, flags);
}

auto coContext::internal::Submission::adviseFile(const std::int32_t fileDescriptor, const std::uint64_t offset,
                                                 const off_t length, const std::int32_t advice) const noexcept -> void {
    io_uring_prep_fadvise64(this->handle, fileDescriptor, offset, length, advice);
}

auto coContext::internal::Submission::truncate(const std::int32_t fileDescriptor, const loff_t length) const noexcept
    -> void {
    io_uring_prep_ftruncate(this->handle, fileDescriptor, length);
}

auto coContext::internal::Submission::allocateFile(const std::int32_t fileDescriptor, const std::int32_t mode,
                                                   const std::uint64_t offset,
                                                   const std::uint64_t length) const noexcept -> void {
    io_uring_prep_fallocate(this->handle, fileDescriptor, mode, offset, length);
}

auto coContext::internal::Submission::status(const std::int32_t directoryFileDescriptor, const std::string_view path,
                                             const std::int32_t flags, const std::uint32_t mask,
                                             struct statx &buffer) const noexcept -> void {
    io_uring_prep_statx(this->handle, directoryFileDescriptor, std::data(path), flags, mask, std::addressof(buffer));
}

auto coContext::internal::Submission::getExtendedAttribute(const std::string_view path, const std::string_view name,
                                                           const std::span<char> value) const noexcept -> void {
    io_uring_prep_getxattr(this->handle, std::data(name), std::data(value), std::data(path), std::size(value));
}

auto coContext::internal::Submission::getExtendedAttribute(const std::int32_t fileDescriptor,
                                                           const std::string_view name,
                                                           const std::span<char> value) const noexcept -> void {
    io_uring_prep_fgetxattr(this->handle, fileDescriptor, std::data(name), std::data(value), std::size(value));
}

auto coContext::internal::Submission::setExtendedAttribute(const std::string_view path, const std::string_view name,
                                                           const std::span<char> value,
                                                           const std::int32_t flags) const noexcept -> void {
    io_uring_prep_setxattr(this->handle, std::data(name), std::data(value), std::data(path), flags, std::size(value));
}

auto coContext::internal::Submission::setExtendedAttribute(const std::int32_t fileDescriptor,
                                                           const std::string_view name, const std::span<char> value,
                                                           const std::int32_t flags) const noexcept -> void {
    io_uring_prep_fsetxattr(this->handle, fileDescriptor, std::data(name), std::data(value), flags, std::size(value));
}

auto coContext::internal::Submission::makeDirectory(const std::string_view path, const mode_t mode) const noexcept
    -> void {
    io_uring_prep_mkdir(this->handle, std::data(path), mode);
}

auto coContext::internal::Submission::makeDirectory(const std::int32_t directoryFileDescriptor,
                                                    const std::string_view path, const mode_t mode) const noexcept
    -> void {
    io_uring_prep_mkdirat(this->handle, directoryFileDescriptor, std::data(path), mode);
}

auto coContext::internal::Submission::rename(const std::string_view oldPath,
                                             const std::string_view newPath) const noexcept -> void {
    io_uring_prep_rename(this->handle, std::data(oldPath), std::data(newPath));
}

auto coContext::internal::Submission::rename(const std::int32_t oldDirectoryFileDescriptor,
                                             const std::string_view oldPath,
                                             const std::int32_t newDirectoryFileDescriptor,
                                             const std::string_view newPath, const std::uint32_t flags) const noexcept
    -> void {
    io_uring_prep_renameat(this->handle, oldDirectoryFileDescriptor, std::data(oldPath), newDirectoryFileDescriptor,
                           std::data(newPath), flags);
}

auto coContext::internal::Submission::link(const std::string_view oldPath, const std::string_view newPath,
                                           const std::int32_t flags) const noexcept -> void {
    io_uring_prep_link(this->handle, std::data(oldPath), std::data(newPath), flags);
}

auto coContext::internal::Submission::link(const std::int32_t oldDirectoryFileDescriptor,
                                           const std::string_view oldPath,
                                           const std::int32_t newDirectoryFileDescriptor,
                                           const std::string_view newPath, const std::int32_t flags) const noexcept
    -> void {
    io_uring_prep_linkat(this->handle, oldDirectoryFileDescriptor, std::data(oldPath), newDirectoryFileDescriptor,
                         std::data(newPath), flags);
}

auto coContext::internal::Submission::symbolicLink(const std::string_view target,
                                                   const std::string_view linkPath) const noexcept -> void {
    io_uring_prep_symlink(this->handle, std::data(target), std::data(linkPath));
}

auto coContext::internal::Submission::symbolicLink(const std::string_view target,
                                                   const std::int32_t newDirectoryFileDescriptor,
                                                   const std::string_view linkPath) const noexcept -> void {
    io_uring_prep_symlinkat(this->handle, std::data(target), newDirectoryFileDescriptor, std::data(linkPath));
}

auto coContext::internal::Submission::unlink(const std::string_view path, const std::int32_t flags) const noexcept
    -> void {
    io_uring_prep_unlink(this->handle, std::data(path), flags);
}

auto coContext::internal::Submission::unlink(const std::int32_t directoryFileDescriptor, const std::string_view path,
                                             const std::int32_t flags) const noexcept -> void {
    io_uring_prep_unlinkat(this->handle, directoryFileDescriptor, std::data(path), flags);
}

auto coContext::internal::Submission::adviseMemory(const std::span<std::byte> buffer,
                                                   const std::int32_t advice) const noexcept -> void {
    io_uring_prep_madvise64(this->handle, std::data(buffer), static_cast<off_t>(std::size(buffer)), advice);
}

auto coContext::internal::Submission::wait(const idtype_t idType, const id_t id, siginfo_t *const signalInformation,
                                           const std::int32_t options, const std::uint32_t flags) const noexcept
    -> void {
    io_uring_prep_waitid(this->handle, idType, id, signalInformation, options, flags);
}

auto coContext::internal::Submission::waitFutex(std::uint32_t &futex, const std::uint64_t value,
                                                const std::uint64_t mask, const std::uint32_t futexFlags,
                                                const std::uint32_t flags) const noexcept -> void {
    io_uring_prep_futex_wait(this->handle, std::addressof(futex), value, mask, futexFlags, flags);
}

auto coContext::internal::Submission::waitFutex(const std::span<futex_waitv> vectorizedFutexs,
                                                const std::uint32_t flags) const noexcept -> void {
    io_uring_prep_futex_waitv(this->handle, std::data(vectorizedFutexs), std::size(vectorizedFutexs), flags);
}

auto coContext::internal::Submission::wakeFutex(std::uint32_t &futex, const std::uint64_t value,
                                                const std::uint64_t mask, const std::uint32_t futexFlags,
                                                const std::uint32_t flags) const noexcept -> void {
    io_uring_prep_futex_wake(this->handle, std::addressof(futex), value, mask, futexFlags, flags);
}

auto coContext::internal::operator==(const Submission lhs, const Submission rhs) noexcept -> bool {
    return lhs.get() == rhs.get();
}

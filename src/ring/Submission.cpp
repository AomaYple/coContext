#include "coContext/ring/Submission.hpp"

auto coContext::internal::Submission::noOperation(io_uring_sqe *const handle) noexcept -> Submission {
    io_uring_prep_nop(handle);

    return Submission{handle};
}

auto coContext::internal::Submission::updateFileDescriptors(io_uring_sqe *const handle,
                                                            const std::span<std::int32_t> fileDescriptors,
                                                            const std::int32_t offset) noexcept -> Submission {
    io_uring_prep_files_update(handle, std::data(fileDescriptors), std::size(fileDescriptors), offset);

    return Submission{handle};
}

auto coContext::internal::Submission::installDirect(io_uring_sqe *const handle, const std::int32_t directFileDescriptor,
                                                    const std::uint32_t flags) noexcept -> Submission {
    io_uring_prep_fixed_fd_install(handle, directFileDescriptor, flags);

    return Submission{handle};
}

auto coContext::internal::Submission::linkTimeout(io_uring_sqe *const handle, __kernel_timespec &timeSpecification,
                                                  const std::uint32_t flags) noexcept -> Submission {
    io_uring_prep_link_timeout(handle, std::addressof(timeSpecification), flags);

    return Submission{handle};
}

auto coContext::internal::Submission::cancel(io_uring_sqe *const handle, const std::uint64_t userData,
                                             const std::int32_t flags) noexcept -> Submission {
    io_uring_prep_cancel64(handle, userData, flags);

    return Submission{handle};
}

auto coContext::internal::Submission::cancel(io_uring_sqe *const handle, const std::int32_t fileDescriptor,
                                             const std::uint32_t flags) noexcept -> Submission {
    io_uring_prep_cancel_fd(handle, fileDescriptor, flags);

    return Submission{handle};
}

auto coContext::internal::Submission::timeout(io_uring_sqe *const handle, __kernel_timespec &timeSpecification,
                                              const std::uint32_t count, const std::uint32_t flags) noexcept
    -> Submission {
    io_uring_prep_timeout(handle, std::addressof(timeSpecification), count, flags);

    return Submission{handle};
}

auto coContext::internal::Submission::updateTimeout(io_uring_sqe *const handle, const std::uint64_t userData,
                                                    __kernel_timespec &timeSpecification,
                                                    const std::uint32_t flags) noexcept -> Submission {
    io_uring_prep_timeout_update(handle, std::addressof(timeSpecification), userData, flags);

    return Submission{handle};
}

auto coContext::internal::Submission::poll(io_uring_sqe *const handle, const std::int32_t fileDescriptor,
                                           const std::uint32_t mask) noexcept -> Submission {
    io_uring_prep_poll_add(handle, fileDescriptor, mask);

    return Submission{handle};
}

auto coContext::internal::Submission::updatePoll(io_uring_sqe *const handle, const std::uint64_t oldUserData,
                                                 const std::uint64_t newUserData, const std::uint32_t mask,
                                                 const std::uint32_t flags) noexcept -> Submission {
    io_uring_prep_poll_update(handle, oldUserData, newUserData, mask, flags);

    return Submission{handle};
}

auto coContext::internal::Submission::multiplePoll(io_uring_sqe *const handle, const std::int32_t fileDescriptor,
                                                   const std::uint32_t mask) noexcept -> Submission {
    io_uring_prep_poll_multishot(handle, fileDescriptor, mask);

    return Submission{handle};
}

auto coContext::internal::Submission::close(io_uring_sqe *const handle, const std::int32_t fileDescriptor) noexcept
    -> Submission {
    io_uring_prep_close(handle, fileDescriptor);

    return Submission{handle};
}

auto coContext::internal::Submission::closeDirect(io_uring_sqe *const handle,
                                                  const std::int32_t directFileDescriptor) noexcept -> Submission {
    io_uring_prep_close_direct(handle, directFileDescriptor);

    return Submission{handle};
}

auto coContext::internal::Submission::socket(io_uring_sqe *const handle, const std::int32_t domain,
                                             const std::int32_t type, const std::int32_t protocol,
                                             const std::uint32_t flags) noexcept -> Submission {
    io_uring_prep_socket(handle, domain, type, protocol, flags);

    return Submission{handle};
}

auto coContext::internal::Submission::directSocket(io_uring_sqe *const handle, const std::int32_t domain,
                                                   const std::int32_t type, const std::int32_t protocol,
                                                   const std::uint32_t flags) noexcept -> Submission {
    io_uring_prep_socket_direct_alloc(handle, domain, type, protocol, flags);

    return Submission{handle};
}

auto coContext::internal::Submission::socketCommand(io_uring_sqe *const handle, const std::int32_t operation,
                                                    const std::int32_t socketFileDescriptor, const std::int32_t level,
                                                    const std::int32_t optionName,
                                                    const std::span<std::byte> option) noexcept -> Submission {
    io_uring_prep_cmd_sock(handle, operation, socketFileDescriptor, level, optionName, std::data(option),
                           static_cast<std::int32_t>(std::size(option)));

    return Submission{handle};
}

auto coContext::internal::Submission::discardCommand(io_uring_sqe *const handle, const std::int32_t fileDescriptor,
                                                     const std::uint64_t offset, const std::uint64_t length) noexcept
    -> Submission {
    io_uring_prep_cmd_discard(handle, fileDescriptor, offset, length);

    return Submission{handle};
}

auto coContext::internal::Submission::bind(io_uring_sqe *const handle, const std::int32_t socketFileDescriptor,
                                           sockaddr &address, const socklen_t addressLength) noexcept -> Submission {
    io_uring_prep_bind(handle, socketFileDescriptor, std::addressof(address), addressLength);

    return Submission{handle};
}

auto coContext::internal::Submission::listen(io_uring_sqe *const handle, const std::int32_t socketFileDescriptor,
                                             const std::int32_t backlog) noexcept -> Submission {
    io_uring_prep_listen(handle, socketFileDescriptor, backlog);

    return Submission{handle};
}

auto coContext::internal::Submission::accept(io_uring_sqe *const handle, const std::int32_t socketFileDescriptor,
                                             sockaddr *const address, socklen_t *const addressLength,
                                             const std::int32_t flags) noexcept -> Submission {
    io_uring_prep_accept(handle, socketFileDescriptor, address, addressLength, flags);

    return Submission{handle};
}

auto coContext::internal::Submission::acceptDirect(io_uring_sqe *const handle, const std::int32_t socketFileDescriptor,
                                                   sockaddr *const address, socklen_t *const addressLength,
                                                   const std::int32_t flags,
                                                   const std::uint32_t fileDescriptorIndex) noexcept -> Submission {
    io_uring_prep_accept_direct(handle, socketFileDescriptor, address, addressLength, flags, fileDescriptorIndex);

    return Submission{handle};
}

auto coContext::internal::Submission::multipleAccept(io_uring_sqe *const handle,
                                                     const std::int32_t socketFileDescriptor, sockaddr *const address,
                                                     socklen_t *const addressLength, const std::int32_t flags) noexcept
    -> Submission {
    io_uring_prep_multishot_accept(handle, socketFileDescriptor, address, addressLength, flags);

    return Submission{handle};
}

auto coContext::internal::Submission::multipleAcceptDirect(io_uring_sqe *const handle,
                                                           const std::int32_t socketFileDescriptor,
                                                           sockaddr *const address, socklen_t *const addressLength,
                                                           const std::int32_t flags) noexcept -> Submission {
    io_uring_prep_multishot_accept_direct(handle, socketFileDescriptor, address, addressLength, flags);

    return Submission{handle};
}

auto coContext::internal::Submission::connect(io_uring_sqe *const handle, const std::int32_t socketFileDescriptor,
                                              const sockaddr &address, const socklen_t addressLength) noexcept
    -> Submission {
    io_uring_prep_connect(handle, socketFileDescriptor, std::addressof(address), addressLength);

    return Submission{handle};
}

auto coContext::internal::Submission::shutdown(io_uring_sqe *const handle, const std::int32_t socketFileDescriptor,
                                               const std::int32_t how) noexcept -> Submission {
    io_uring_prep_shutdown(handle, socketFileDescriptor, how);

    return Submission{handle};
}

auto coContext::internal::Submission::receive(io_uring_sqe *const handle, const std::int32_t socketFileDescriptor,
                                              const std::span<std::byte> buffer, const std::int32_t flags) noexcept
    -> Submission {
    io_uring_prep_recv(handle, socketFileDescriptor, std::data(buffer), std::size(buffer), flags);

    return Submission{handle};
}

auto coContext::internal::Submission::receive(io_uring_sqe *const handle, const std::int32_t socketFileDescriptor,
                                              msghdr &message, const std::uint32_t flags) noexcept -> Submission {
    io_uring_prep_recvmsg(handle, socketFileDescriptor, std::addressof(message), flags);

    return Submission{handle};
}

auto coContext::internal::Submission::multipleReceive(io_uring_sqe *const handle,
                                                      const std::int32_t socketFileDescriptor,
                                                      const std::span<std::byte> buffer,
                                                      const std::int32_t flags) noexcept -> Submission {
    io_uring_prep_recv_multishot(handle, socketFileDescriptor, std::data(buffer), std::size(buffer), flags);

    return Submission{handle};
}

auto coContext::internal::Submission::send(io_uring_sqe *const handle, const std::int32_t socketFileDescriptor,
                                           const std::span<const std::byte> buffer, const std::int32_t flags) noexcept
    -> Submission {
    io_uring_prep_send(handle, socketFileDescriptor, std::data(buffer), std::size(buffer), flags);

    return Submission{handle};
}

auto coContext::internal::Submission::send(io_uring_sqe *const handle, const std::int32_t socketFileDescriptor,
                                           const std::span<const std::byte> buffer, const std::int32_t flags,
                                           const sockaddr &destinationAddress,
                                           const socklen_t destinationAddressLength) noexcept -> Submission {
    io_uring_prep_sendto(handle, socketFileDescriptor, std::data(buffer), std::size(buffer), flags,
                         std::addressof(destinationAddress), destinationAddressLength);

    return Submission{handle};
}

auto coContext::internal::Submission::send(io_uring_sqe *const handle, const std::int32_t socketFileDescriptor,
                                           const msghdr &message, const std::uint32_t flags) noexcept -> Submission {
    io_uring_prep_sendmsg(handle, socketFileDescriptor, std::addressof(message), flags);

    return Submission{handle};
}

auto coContext::internal::Submission::zeroCopySend(io_uring_sqe *const handle, const std::int32_t socketFileDescriptor,
                                                   const std::span<const std::byte> buffer, const std::int32_t flags,
                                                   const std::uint32_t zeroCopyFlags) noexcept -> Submission {
    io_uring_prep_send_zc(handle, socketFileDescriptor, std::data(buffer), std::size(buffer), flags, zeroCopyFlags);

    return Submission{handle};
}

auto coContext::internal::Submission::zeroCopySend(io_uring_sqe *const handle, const std::int32_t socketFileDescriptor,
                                                   const msghdr &message, const std::uint32_t flags) noexcept
    -> Submission {
    io_uring_prep_sendmsg_zc(handle, socketFileDescriptor, std::addressof(message), flags);

    return Submission{handle};
}

auto coContext::internal::Submission::splice(io_uring_sqe *const handle, const std::int32_t inFileDescriptor,
                                             const std::int64_t inOffset, const std::int32_t outFileDescriptor,
                                             const std::int64_t outOffset, const std::uint32_t length,
                                             const std::uint32_t flags) noexcept -> Submission {
    io_uring_prep_splice(handle, inFileDescriptor, inOffset, outFileDescriptor, outOffset, length, flags);

    return Submission{handle};
}

auto coContext::internal::Submission::tee(io_uring_sqe *const handle, const std::int32_t inFileDescriptor,
                                          const std::int32_t outFileDescriptor, const std::uint32_t length,
                                          const std::uint32_t flags) noexcept -> Submission {
    io_uring_prep_tee(handle, inFileDescriptor, outFileDescriptor, length, flags);

    return Submission{handle};
}

auto coContext::internal::Submission::open(io_uring_sqe *const handle, const std::filesystem::path &path,
                                           const std::int32_t flags, const mode_t mode) noexcept -> Submission {
    io_uring_prep_open(handle, path.c_str(), flags, mode);

    return Submission{handle};
}

auto coContext::internal::Submission::open(io_uring_sqe *const handle, const std::int32_t directoryFileDescriptor,
                                           const std::filesystem::path &path, const std::int32_t flags,
                                           const mode_t mode) noexcept -> Submission {
    io_uring_prep_openat(handle, directoryFileDescriptor, path.c_str(), flags, mode);

    return Submission{handle};
}

auto coContext::internal::Submission::open(io_uring_sqe *const handle, const std::int32_t directoryFileDescriptor,
                                           const std::filesystem::path &path, open_how &openHow) noexcept
    -> Submission {
    io_uring_prep_openat2(handle, directoryFileDescriptor, path.c_str(), std::addressof(openHow));

    return Submission{handle};
}

auto coContext::internal::Submission::openDirect(io_uring_sqe *const handle, const std::filesystem::path &path,
                                                 const std::int32_t flags, const mode_t mode,
                                                 const std::uint32_t fileDescriptorIndex) noexcept -> Submission {
    io_uring_prep_open_direct(handle, path.c_str(), flags, mode, fileDescriptorIndex);

    return Submission{handle};
}

auto coContext::internal::Submission::openDirect(io_uring_sqe *const handle, const std::int32_t directoryFileDescriptor,
                                                 const std::filesystem::path &path, const std::int32_t flags,
                                                 const mode_t mode, const std::uint32_t fileDescriptorIndex) noexcept
    -> Submission {
    io_uring_prep_openat_direct(handle, directoryFileDescriptor, path.c_str(), flags, mode, fileDescriptorIndex);

    return Submission{handle};
}

auto coContext::internal::Submission::openDirect(io_uring_sqe *const handle, const std::int32_t directoryFileDescriptor,
                                                 const std::filesystem::path &path, open_how &openHow,
                                                 const std::uint32_t fileDescriptorIndex) noexcept -> Submission {
    io_uring_prep_openat2_direct(handle, directoryFileDescriptor, path.c_str(), std::addressof(openHow),
                                 fileDescriptorIndex);

    return Submission{handle};
}

auto coContext::internal::Submission::read(io_uring_sqe *const handle, const std::int32_t fileDescriptor,
                                           const std::span<std::byte> buffer, const std::uint64_t offset) noexcept
    -> Submission {
    io_uring_prep_read(handle, fileDescriptor, std::data(buffer), std::size(buffer), offset);

    return Submission{handle};
}

auto coContext::internal::Submission::read(io_uring_sqe *const handle, const std::int32_t fileDescriptor,
                                           const std::span<const iovec> buffer, const std::uint64_t offset) noexcept
    -> Submission {
    io_uring_prep_readv(handle, fileDescriptor, std::data(buffer), std::size(buffer), offset);

    return Submission{handle};
}

auto coContext::internal::Submission::read(io_uring_sqe *const handle, const std::int32_t fileDescriptor,
                                           const std::span<const iovec> buffer, const std::uint64_t offset,
                                           const std::int32_t flags) noexcept -> Submission {
    io_uring_prep_readv2(handle, fileDescriptor, std::data(buffer), std::size(buffer), offset, flags);

    return Submission{handle};
}

auto coContext::internal::Submission::multipleRead(io_uring_sqe *const handle, const std::int32_t fileDescriptor,
                                                   const std::uint32_t length, const std::uint64_t offset,
                                                   const std::int32_t bufferGroup) noexcept -> Submission {
    io_uring_prep_read_multishot(handle, fileDescriptor, length, offset, bufferGroup);

    return Submission{handle};
}

auto coContext::internal::Submission::write(io_uring_sqe *const handle, const std::int32_t fileDescriptor,
                                            const std::span<const std::byte> buffer,
                                            const std::uint64_t offset) noexcept -> Submission {
    io_uring_prep_write(handle, fileDescriptor, std::data(buffer), std::size(buffer), offset);

    return Submission{handle};
}

auto coContext::internal::Submission::write(io_uring_sqe *const handle, const std::int32_t fileDescriptor,
                                            const std::span<const iovec> buffer, const std::uint64_t offset) noexcept
    -> Submission {
    io_uring_prep_writev(handle, fileDescriptor, std::data(buffer), std::size(buffer), offset);

    return Submission{handle};
}

auto coContext::internal::Submission::write(io_uring_sqe *const handle, const std::int32_t fileDescriptor,
                                            const std::span<const iovec> buffer, const std::uint64_t offset,
                                            const std::int32_t flags) noexcept -> Submission {
    io_uring_prep_writev2(handle, fileDescriptor, std::data(buffer), std::size(buffer), offset, flags);

    return Submission{handle};
}

auto coContext::internal::Submission::syncFile(io_uring_sqe *const handle, const std::int32_t fileDescriptor,
                                               const std::uint32_t flags) noexcept -> Submission {
    io_uring_prep_fsync(handle, fileDescriptor, flags);

    return Submission{handle};
}

auto coContext::internal::Submission::syncFile(io_uring_sqe *const handle, const std::int32_t fileDescriptor,
                                               const std::uint64_t offset, const std::uint32_t length,
                                               const std::int32_t flags) noexcept -> Submission {
    io_uring_prep_sync_file_range(handle, fileDescriptor, length, offset, flags);

    return Submission{handle};
}

auto coContext::internal::Submission::adviseFile(io_uring_sqe *const handle, const std::int32_t fileDescriptor,
                                                 const std::uint64_t offset, const off_t length,
                                                 const std::int32_t advice) noexcept -> Submission {
    io_uring_prep_fadvise64(handle, fileDescriptor, offset, length, advice);

    return Submission{handle};
}

auto coContext::internal::Submission::truncate(io_uring_sqe *const handle, const std::int32_t fileDescriptor,
                                               const loff_t length) noexcept -> Submission {
    io_uring_prep_ftruncate(handle, fileDescriptor, length);

    return Submission{handle};
}

auto coContext::internal::Submission::allocateFile(io_uring_sqe *const handle, const std::int32_t fileDescriptor,
                                                   const std::int32_t mode, const std::uint64_t offset,
                                                   const std::uint64_t length) noexcept -> Submission {
    io_uring_prep_fallocate(handle, fileDescriptor, mode, offset, length);

    return Submission{handle};
}

auto coContext::internal::Submission::status(io_uring_sqe *const handle, const std::int32_t directoryFileDescriptor,
                                             const std::filesystem::path &path, const std::int32_t flags,
                                             const std::uint32_t mask, struct statx &buffer) noexcept -> Submission {
    io_uring_prep_statx(handle, directoryFileDescriptor, path.c_str(), flags, mask, std::addressof(buffer));

    return Submission{handle};
}

auto coContext::internal::Submission::getExtendedAttribute(io_uring_sqe *const handle,
                                                           const std::filesystem::path &path,
                                                           const std::string_view name,
                                                           const std::span<char> value) noexcept -> Submission {
    io_uring_prep_getxattr(handle, std::data(name), std::data(value), path.c_str(), std::size(value));

    return Submission{handle};
}

auto coContext::internal::Submission::getExtendedAttribute(io_uring_sqe *const handle,
                                                           const std::int32_t fileDescriptor,
                                                           const std::string_view name,
                                                           const std::span<char> value) noexcept -> Submission {
    io_uring_prep_fgetxattr(handle, fileDescriptor, std::data(name), std::data(value), std::size(value));

    return Submission{handle};
}

auto coContext::internal::Submission::setExtendedAttribute(io_uring_sqe *const handle,
                                                           const std::filesystem::path &path,
                                                           const std::string_view name, const std::span<char> value,
                                                           const std::int32_t flags) noexcept -> Submission {
    io_uring_prep_setxattr(handle, std::data(name), std::data(value), path.c_str(), flags, std::size(value));

    return Submission{handle};
}

auto coContext::internal::Submission::setExtendedAttribute(io_uring_sqe *const handle,
                                                           const std::int32_t fileDescriptor,
                                                           const std::string_view name, const std::span<char> value,
                                                           const std::int32_t flags) noexcept -> Submission {
    io_uring_prep_fsetxattr(handle, fileDescriptor, std::data(name), std::data(value), flags, std::size(value));

    return Submission{handle};
}

auto coContext::internal::Submission::makeDirectory(io_uring_sqe *const handle, const std::filesystem::path &path,
                                                    const mode_t mode) noexcept -> Submission {
    io_uring_prep_mkdir(handle, path.c_str(), mode);

    return Submission{handle};
}

auto coContext::internal::Submission::makeDirectory(io_uring_sqe *const handle,
                                                    const std::int32_t directoryFileDescriptor,
                                                    const std::filesystem::path &path, const mode_t mode) noexcept
    -> Submission {
    io_uring_prep_mkdirat(handle, directoryFileDescriptor, path.c_str(), mode);

    return Submission{handle};
}

auto coContext::internal::Submission::rename(io_uring_sqe *const handle, const std::filesystem::path &oldPath,
                                             const std::filesystem::path &newPath) noexcept -> Submission {
    io_uring_prep_rename(handle, oldPath.c_str(), newPath.c_str());

    return Submission{handle};
}

auto coContext::internal::Submission::rename(io_uring_sqe *const handle, const std::int32_t oldDirectoryFileDescriptor,
                                             const std::filesystem::path &oldPath,
                                             const std::int32_t newDirectoryFileDescriptor,
                                             const std::filesystem::path &newPath, const std::uint32_t flags) noexcept
    -> Submission {
    io_uring_prep_renameat(handle, oldDirectoryFileDescriptor, oldPath.c_str(), newDirectoryFileDescriptor,
                           newPath.c_str(), flags);

    return Submission{handle};
}

auto coContext::internal::Submission::link(io_uring_sqe *const handle, const std::filesystem::path &oldPath,
                                           const std::filesystem::path &newPath, const std::int32_t flags) noexcept
    -> Submission {
    io_uring_prep_link(handle, oldPath.c_str(), newPath.c_str(), flags);

    return Submission{handle};
}

auto coContext::internal::Submission::link(io_uring_sqe *const handle, const std::int32_t oldDirectoryFileDescriptor,
                                           const std::filesystem::path &oldPath,
                                           const std::int32_t newDirectoryFileDescriptor,
                                           const std::filesystem::path &newPath, const std::int32_t flags) noexcept
    -> Submission {
    io_uring_prep_linkat(handle, oldDirectoryFileDescriptor, oldPath.c_str(), newDirectoryFileDescriptor,
                         newPath.c_str(), flags);

    return Submission{handle};
}

auto coContext::internal::Submission::symbolicLink(io_uring_sqe *const handle, const std::string_view target,
                                                   const std::filesystem::path &linkPath) noexcept -> Submission {
    io_uring_prep_symlink(handle, std::data(target), linkPath.c_str());

    return Submission{handle};
}

auto coContext::internal::Submission::symbolicLink(io_uring_sqe *const handle, const std::string_view target,
                                                   const std::int32_t newDirectoryFileDescriptor,
                                                   const std::filesystem::path &linkPath) noexcept -> Submission {
    io_uring_prep_symlinkat(handle, std::data(target), newDirectoryFileDescriptor, linkPath.c_str());

    return Submission{handle};
}

auto coContext::internal::Submission::unlink(io_uring_sqe *const handle, const std::filesystem::path &path,
                                             const std::int32_t flags) noexcept -> Submission {
    io_uring_prep_unlink(handle, path.c_str(), flags);

    return Submission{handle};
}

auto coContext::internal::Submission::unlink(io_uring_sqe *const handle, const std::int32_t directoryFileDescriptor,
                                             const std::filesystem::path &path, const std::int32_t flags) noexcept
    -> Submission {
    io_uring_prep_unlinkat(handle, directoryFileDescriptor, path.c_str(), flags);

    return Submission{handle};
}

auto coContext::internal::Submission::adviseMemory(io_uring_sqe *const handle, const std::span<std::byte> buffer,
                                                   const std::int32_t advice) noexcept -> Submission {
    io_uring_prep_madvise64(handle, std::data(buffer), static_cast<off_t>(std::size(buffer)), advice);

    return Submission{handle};
}

auto coContext::internal::Submission::wait(io_uring_sqe *const handle, const idtype_t idType, const id_t id,
                                           siginfo_t *const signalInformation, const std::int32_t options,
                                           const std::uint32_t flags) noexcept -> Submission {
    io_uring_prep_waitid(handle, idType, id, signalInformation, options, flags);

    return Submission{handle};
}

auto coContext::internal::Submission::waitFutex(io_uring_sqe *const handle, std::uint32_t &futex,
                                                const std::uint64_t value, const std::uint64_t mask,
                                                const std::uint32_t futexFlags, const std::uint32_t flags) noexcept
    -> Submission {
    io_uring_prep_futex_wait(handle, std::addressof(futex), value, mask, futexFlags, flags);

    return Submission{handle};
}

auto coContext::internal::Submission::waitFutex(io_uring_sqe *const handle,
                                                const std::span<futex_waitv> vectorizedFutexs,
                                                const std::uint32_t flags) noexcept -> Submission {
    io_uring_prep_futex_waitv(handle, std::data(vectorizedFutexs), std::size(vectorizedFutexs), flags);

    return Submission{handle};
}

auto coContext::internal::Submission::wakeFutex(io_uring_sqe *const handle, std::uint32_t &futex,
                                                const std::uint64_t value, const std::uint64_t mask,
                                                const std::uint32_t futexFlags, const std::uint32_t flags) noexcept
    -> Submission {
    io_uring_prep_futex_wake(handle, std::addressof(futex), value, mask, futexFlags, flags);

    return Submission{handle};
}

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

auto coContext::internal::operator==(const Submission lhs, const Submission rhs) noexcept -> bool {
    return lhs.get() == rhs.get();
}

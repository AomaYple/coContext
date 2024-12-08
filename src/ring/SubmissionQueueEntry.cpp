#include "coContext/ring/SubmissionQueueEntry.hpp"

#include <liburing.h>

coContext::SubmissionQueueEntry::SubmissionQueueEntry(io_uring_sqe *const handle) noexcept : handle{handle} {}

coContext::SubmissionQueueEntry::operator bool() const noexcept { return this->handle != nullptr; }

auto coContext::SubmissionQueueEntry::addFlags(const std::uint32_t flags) const noexcept -> void {
    io_uring_sqe_set_flags(this->handle, this->handle->flags | flags);
}

auto coContext::SubmissionQueueEntry::addIoPriority(const std::uint16_t ioPriority) const noexcept -> void {
    this->handle->ioprio |= ioPriority;
}

auto coContext::SubmissionQueueEntry::setUserData(const std::uint64_t userData) const noexcept -> void {
    io_uring_sqe_set_data64(this->handle, userData);
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

auto coContext::SubmissionQueueEntry::updateTimeout(__kernel_timespec &timeSpecification, const std::uint64_t userData,
                                                    const std::uint32_t flags) const noexcept -> void {
    io_uring_prep_timeout_update(this->handle, std::addressof(timeSpecification), userData, flags);
}

auto coContext::SubmissionQueueEntry::removeTimeout(const std::uint64_t userData,
                                                    const std::uint32_t flags) const noexcept -> void {
    io_uring_prep_timeout_remove(this->handle, userData, flags);
}

auto coContext::SubmissionQueueEntry::close(const std::int32_t fileDescriptor) const noexcept -> void {
    io_uring_prep_close(this->handle, fileDescriptor);
}

auto coContext::SubmissionQueueEntry::socket(const std::int32_t domain, const std::int32_t type,
                                             const std::int32_t protocol, const std::uint32_t flags) const noexcept
    -> void {
    io_uring_prep_socket(this->handle, domain, type, protocol, flags);
}

auto coContext::SubmissionQueueEntry::bind(const std::int32_t socketFileDescriptor, sockaddr *const address,
                                           const std::uint32_t addressLength) const noexcept -> void {
    io_uring_prep_bind(this->handle, socketFileDescriptor, address, addressLength);
}

auto coContext::SubmissionQueueEntry::listen(const std::int32_t socketFileDescriptor,
                                             const std::int32_t backlog) const noexcept -> void {
    io_uring_prep_listen(this->handle, socketFileDescriptor, backlog);
}

auto coContext::SubmissionQueueEntry::accept(const std::int32_t socketFileDescriptor, sockaddr *const address,
                                             std::uint32_t *const addressLength,
                                             const std::int32_t flags) const noexcept -> void {
    io_uring_prep_accept(this->handle, socketFileDescriptor, address, addressLength, flags);
}

auto coContext::SubmissionQueueEntry::connect(const std::int32_t socketFileDescriptor, const sockaddr *const address,
                                              const std::uint32_t addressLength) const noexcept -> void {
    io_uring_prep_connect(this->handle, socketFileDescriptor, address, addressLength);
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
                                           const sockaddr *const address,
                                           const std::uint32_t addressLength) const noexcept -> void {
    io_uring_prep_sendto(this->handle, socketFileDescriptor, std::data(buffer), std::size(buffer), flags, address,
                         addressLength);
}

auto coContext::SubmissionQueueEntry::send(const std::int32_t socketFileDescriptor, const msghdr &message,
                                           const std::uint32_t flags) const noexcept -> void {
    io_uring_prep_sendmsg(this->handle, socketFileDescriptor, std::addressof(message), flags);
}

auto coContext::SubmissionQueueEntry::open(const std::string_view pathname, const std::int32_t flags,
                                           const std::uint32_t mode) const noexcept -> void {
    io_uring_prep_open(this->handle, std::data(pathname), flags, mode);
}

auto coContext::SubmissionQueueEntry::open(const std::int32_t directoryFileDescriptor, const std::string_view pathname,
                                           const std::int32_t flags, const std::uint32_t mode) const noexcept -> void {
    io_uring_prep_openat(this->handle, directoryFileDescriptor, std::data(pathname), flags, mode);
}

auto coContext::SubmissionQueueEntry::open(const std::int32_t directoryFileDescriptor, const std::string_view pathname,
                                           open_how &how) const noexcept -> void {
    io_uring_prep_openat2(this->handle, directoryFileDescriptor, std::data(pathname), std::addressof(how));
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

auto coContext::operator==(const SubmissionQueueEntry lhs, const SubmissionQueueEntry rhs) noexcept -> bool {
    return lhs.handle == rhs.handle;
}

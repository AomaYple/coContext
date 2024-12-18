#pragma once

#include <cstdint>
#include <liburing/io_uring.h>
#include <linux/openat2.h>
#include <span>
#include <string_view>
#include <sys/socket.h>
#include <sys/stat.h>

namespace coContext {
    class SubmissionQueueEntry {
    public:
        explicit SubmissionQueueEntry(io_uring_sqe *handle = {}) noexcept;

        constexpr SubmissionQueueEntry(const SubmissionQueueEntry &) noexcept = default;

        constexpr auto operator=(const SubmissionQueueEntry &) noexcept -> SubmissionQueueEntry & = default;

        constexpr SubmissionQueueEntry(SubmissionQueueEntry &&) noexcept = default;

        constexpr auto operator=(SubmissionQueueEntry &&) noexcept -> SubmissionQueueEntry & = default;

        constexpr ~SubmissionQueueEntry() = default;

        [[nodiscard]] auto get() const noexcept -> io_uring_sqe *;

        auto addFlags(std::uint32_t flags) const noexcept -> void;

        auto addIoPriority(std::uint16_t ioPriority) const noexcept -> void;

        auto setUserData(std::uint64_t userData) const noexcept -> void;

        auto cancel(std::uint64_t userData, std::int32_t flags) const noexcept -> void;

        auto cancel(std::int32_t fileDescriptor, std::uint32_t flags) const noexcept -> void;

        auto timeout(__kernel_timespec &timeSpecification, std::uint32_t count, std::uint32_t flags) const noexcept
            -> void;

        auto updateTimeout(__kernel_timespec &timeSpecification, std::uint64_t userData,
                           std::uint32_t flags) const noexcept -> void;

        auto linkTimeout(__kernel_timespec &timeSpecification, std::uint32_t flags) const noexcept -> void;

        auto close(std::int32_t fileDescriptor) const noexcept -> void;

        auto socket(std::int32_t domain, std::int32_t type, std::int32_t protocol, std::uint32_t flags) const noexcept
            -> void;

        auto bind(std::int32_t socketFileDescriptor, sockaddr *address, socklen_t addressLength) const noexcept -> void;

        auto listen(std::int32_t socketFileDescriptor, std::int32_t backlog) const noexcept -> void;

        auto accept(std::int32_t socketFileDescriptor, sockaddr *address, socklen_t *addressLength,
                    std::int32_t flags) const noexcept -> void;

        auto connect(std::int32_t socketFileDescriptor, const sockaddr *address, socklen_t addressLength) const noexcept
            -> void;

        auto shutdown(std::int32_t socketFileDescriptor, std::int32_t how) const noexcept -> void;

        auto receive(std::int32_t socketFileDescriptor, std::span<std::byte> buffer, std::int32_t flags) const noexcept
            -> void;

        auto receive(std::int32_t socketFileDescriptor, msghdr &message, std::uint32_t flags) const noexcept -> void;

        auto send(std::int32_t socketFileDescriptor, std::span<const std::byte> buffer,
                  std::int32_t flags) const noexcept -> void;

        auto send(std::int32_t socketFileDescriptor, std::span<const std::byte> buffer, std::int32_t flags,
                  const sockaddr *destinationAddress, socklen_t destinationAddressLength) const noexcept -> void;

        auto send(std::int32_t socketFileDescriptor, const msghdr &message, std::uint32_t flags) const noexcept -> void;

        auto open(std::string_view path, std::int32_t flags, mode_t mode) const noexcept -> void;

        auto open(std::int32_t directoryFileDescriptor, std::string_view path, std::int32_t flags,
                  mode_t mode) const noexcept -> void;

        auto open(std::int32_t directoryFileDescriptor, std::string_view path, open_how &openHow) const noexcept
            -> void;

        auto read(std::int32_t fileDescriptor, std::span<std::byte> buffer, std::uint64_t offset) const noexcept
            -> void;

        auto read(std::int32_t fileDescriptor, std::span<const iovec> buffer, std::uint64_t offset) const noexcept
            -> void;

        auto read(std::int32_t fileDescriptor, std::span<const iovec> buffer, std::uint64_t offset,
                  std::int32_t flags) const noexcept -> void;

        auto write(std::int32_t fileDescriptor, std::span<const std::byte> buffer, std::uint64_t offset) const noexcept
            -> void;

        auto write(std::int32_t fileDescriptor, std::span<const iovec> buffer, std::uint64_t offset) const noexcept
            -> void;

        auto write(std::int32_t fileDescriptor, std::span<const iovec> buffer, std::uint64_t offset,
                   std::int32_t flags) const noexcept -> void;

        auto syncFile(std::int32_t fileDescriptor, std::uint32_t flags) const noexcept -> void;

        auto syncFile(std::int32_t fileDescriptor, std::uint64_t offset, std::uint32_t length,
                      std::int32_t flags) const noexcept -> void;

        auto link(std::string_view oldPath, std::string_view newPath, std::int32_t flags) const noexcept -> void;

        auto link(std::int32_t oldDirectoryFileDescriptor, std::string_view oldPath,
                  std::int32_t newDirectoryFileDescriptor, std::string_view newPath, std::int32_t flags) const noexcept
            -> void;

        auto symbolicLink(std::string_view target, std::string_view linkPath) const noexcept -> void;

        auto symbolicLink(std::string_view target, std::int32_t newDirectoryFileDescriptor,
                          std::string_view linkPath) const noexcept -> void;

        auto unlink(std::string_view path, std::int32_t flags) const noexcept -> void;

        auto unlink(std::int32_t directoryFileDescriptor, std::string_view path, std::int32_t flags) const noexcept
            -> void;

        auto makeDirectory(std::string_view path, mode_t mode) const noexcept -> void;

        auto makeDirectory(std::int32_t directoryFileDescriptor, std::string_view path, mode_t mode) const noexcept
            -> void;

        auto rename(std::string_view oldPath, std::string_view newPath) const noexcept -> void;

        auto rename(std::int32_t oldDirectoryFileDescriptor, std::string_view oldPath,
                    std::int32_t newDirectoryFileDescriptor, std::string_view newPath,
                    std::uint32_t flags) const noexcept -> void;

        auto truncate(std::int32_t fileDescriptor, loff_t length) const noexcept -> void;

        auto allocateFile(std::int32_t fileDescriptor, std::int32_t mode, std::uint64_t offset,
                          std::uint64_t length) const noexcept -> void;

        auto getFileStatus(std::int32_t directoryFileDescriptor, std::string_view path, std::int32_t flags,
                           std::uint32_t mask, struct statx &buffer) const noexcept -> void;

        auto getExtendedAttribute(std::string_view path, std::string_view name, std::span<char> value) const noexcept
            -> void;

        auto getExtendedAttribute(std::int32_t fileDescriptor, std::string_view name,
                                  std::span<char> value) const noexcept -> void;

        auto setExtendedAttribute(std::string_view path, std::string_view name, std::span<char> value,
                                  std::int32_t flags) const noexcept -> void;

        auto setExtendedAttribute(std::int32_t fileDescriptor, std::string_view name, std::span<char> value,
                                  std::int32_t flags) const noexcept -> void;

        auto adviseMemory(std::span<std::byte> buffer, std::int32_t advice) const noexcept -> void;

        auto adviseFile(std::int32_t fileDescriptor, std::uint64_t offset, off_t length,
                        std::int32_t advice) const noexcept -> void;

        auto splice(std::int32_t inFileDescriptor, std::int64_t inFileDescriptorOffset, std::int32_t outFileDescriptor,
                    std::int64_t outFileDescriptorOffset, std::uint32_t numberOfBytes,
                    std::uint32_t flags) const noexcept -> void;

    private:
        io_uring_sqe *handle;
    };

    [[nodiscard]] auto operator==(SubmissionQueueEntry, SubmissionQueueEntry) noexcept -> bool;
}    // namespace coContext

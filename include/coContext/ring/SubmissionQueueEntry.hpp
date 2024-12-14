#pragma once

#include <cstdint>
#include <liburing/io_uring.h>
#include <linux/openat2.h>
#include <span>
#include <string_view>
#include <sys/socket.h>

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

        auto socket(std::int32_t domain, std::int32_t type, std::int32_t protocol,
                    std::uint32_t flags = {}) const noexcept -> void;

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
                  const sockaddr *address, socklen_t addressLength) const noexcept -> void;

        auto send(std::int32_t socketFileDescriptor, const msghdr &message, std::uint32_t flags) const noexcept -> void;

        auto open(std::string_view pathname, std::int32_t flags, mode_t mode) const noexcept -> void;

        auto open(std::int32_t directoryFileDescriptor, std::string_view pathname, std::int32_t flags,
                  mode_t mode) const noexcept -> void;

        auto open(std::int32_t directoryFileDescriptor, std::string_view pathname, open_how &how) const noexcept
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

        auto link(std::string_view oldPath, std::string_view newPath, std::int32_t flags) const noexcept -> void;

        auto link(std::int32_t oldDirectoryFileDescriptor, std::string_view oldPath,
                  std::int32_t newDirectoryFileDescriptor, std::string_view newPath, std::int32_t flags) const noexcept
            -> void;

    private:
        io_uring_sqe *handle;
    };

    [[nodiscard]] auto operator==(SubmissionQueueEntry lhs, SubmissionQueueEntry rhs) noexcept -> bool;
}    // namespace coContext

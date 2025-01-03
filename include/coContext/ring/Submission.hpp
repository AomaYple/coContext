#pragma once

#include <liburing.h>
#include <span>
#include <string_view>

namespace coContext::internal {
    class Submission {
    public:
        explicit Submission(io_uring_sqe *handle = {}) noexcept;

        constexpr Submission(const Submission &) noexcept = default;

        constexpr auto operator=(const Submission &) noexcept -> Submission & = default;

        constexpr Submission(Submission &&) noexcept = default;

        constexpr auto operator=(Submission &&) noexcept -> Submission & = default;

        constexpr ~Submission() = default;

        [[nodiscard]] auto get() const noexcept -> io_uring_sqe *;

        auto addFlags(std::uint32_t flags) const noexcept -> void;

        auto addIoPriority(std::uint16_t ioPriority) const noexcept -> void;

        auto setUserData(std::uint64_t userData) const noexcept -> void;

        auto setBufferGroup(std::uint16_t bufferGroup) const noexcept -> void;

        auto linkTimeout(__kernel_timespec &timeSpecification, std::uint32_t flags) const noexcept -> void;

        auto cancel(std::uint64_t userData, std::int32_t flags) const noexcept -> void;

        auto cancel(std::int32_t fileDescriptor, std::uint32_t flags) const noexcept -> void;

        auto timeout(__kernel_timespec &timeSpecification, std::uint32_t count, std::uint32_t flags) const noexcept
            -> void;

        auto updateTimeout(std::uint64_t userData, __kernel_timespec &timeSpecification,
                           std::uint32_t flags) const noexcept -> void;

        auto poll(std::int32_t fileDescriptor, std::uint32_t mask) const noexcept -> void;

        auto updatePoll(std::uint64_t oldUserData, std::uint64_t newUserData, std::uint32_t mask,
                        std::uint32_t flags) const noexcept -> void;

        auto multiplePoll(std::int32_t fileDescriptor, std::uint32_t mask) const noexcept -> void;

        auto installDirect(std::int32_t directFileDescriptor, std::uint32_t flags) const noexcept -> void;

        auto close(std::int32_t fileDescriptor) const noexcept -> void;

        auto closeDirect(std::int32_t directFileDescriptor) const noexcept -> void;

        auto socket(std::int32_t domain, std::int32_t type, std::int32_t protocol, std::uint32_t flags) const noexcept
            -> void;

        auto directSocket(std::int32_t domain, std::int32_t type, std::int32_t protocol,
                          std::uint32_t flags) const noexcept -> void;

        auto socketCommand(std::int32_t operation, std::int32_t socketFileDescriptor, std::int32_t level,
                           std::int32_t optionName, std::span<std::byte> option) const noexcept -> void;

        auto discardCommand(std::int32_t fileDescriptor, std::uint64_t offset, std::uint64_t length) const noexcept
            -> void;

        auto bind(std::int32_t socketFileDescriptor, sockaddr &address, socklen_t addressLength) const noexcept -> void;

        auto listen(std::int32_t socketFileDescriptor, std::int32_t backlog) const noexcept -> void;

        auto accept(std::int32_t socketFileDescriptor, sockaddr *address, socklen_t *addressLength,
                    std::int32_t flags) const noexcept -> void;

        auto acceptDirect(std::int32_t socketFileDescriptor, sockaddr *address, socklen_t *addressLength,
                          std::int32_t flags, std::uint32_t fileDescriptorIndex) const noexcept -> void;

        auto multipleAccept(std::int32_t socketFileDescriptor, sockaddr *address, socklen_t *addressLength,
                            std::int32_t flags) const noexcept -> void;

        auto multipleAcceptDirect(std::int32_t socketFileDescriptor, sockaddr *address, socklen_t *addressLength,
                                  std::int32_t flags) const noexcept -> void;

        auto connect(std::int32_t socketFileDescriptor, const sockaddr &address, socklen_t addressLength) const noexcept
            -> void;

        auto shutdown(std::int32_t socketFileDescriptor, std::int32_t how) const noexcept -> void;

        auto receive(std::int32_t socketFileDescriptor, std::span<std::byte> buffer, std::int32_t flags) const noexcept
            -> void;

        auto receive(std::int32_t socketFileDescriptor, msghdr &message, std::uint32_t flags) const noexcept -> void;

        auto multipleReceive(std::int32_t socketFileDescriptor, std::span<std::byte> buffer,
                             std::int32_t flags) const noexcept -> void;

        auto send(std::int32_t socketFileDescriptor, std::span<const std::byte> buffer,
                  std::int32_t flags) const noexcept -> void;

        auto send(std::int32_t socketFileDescriptor, std::span<const std::byte> buffer, std::int32_t flags,
                  const sockaddr &destinationAddress, socklen_t destinationAddressLength) const noexcept -> void;

        auto send(std::int32_t socketFileDescriptor, const msghdr &message, std::uint32_t flags) const noexcept -> void;

        auto splice(std::int32_t inFileDescriptor, std::int64_t inFileDescriptorOffset, std::int32_t outFileDescriptor,
                    std::int64_t outFileDescriptorOffset, std::uint32_t length, std::uint32_t flags) const noexcept
            -> void;

        auto tee(std::int32_t inFileDescriptor, std::int32_t outFileDescriptor, std::uint32_t length,
                 std::uint32_t flags) const noexcept -> void;

        auto open(std::string_view path, std::int32_t flags, mode_t mode) const noexcept -> void;

        auto open(std::int32_t directoryFileDescriptor, std::string_view path, std::int32_t flags,
                  mode_t mode) const noexcept -> void;

        auto open(std::int32_t directoryFileDescriptor, std::string_view path, open_how &openHow) const noexcept
            -> void;

        auto openDirect(std::string_view path, std::int32_t flags, mode_t mode,
                        std::uint32_t fileDescriptorIndex) const noexcept -> void;

        auto openDirect(std::int32_t directoryFileDescriptor, std::string_view path, std::int32_t flags, mode_t mode,
                        std::uint32_t fileDescriptorIndex) const noexcept -> void;

        auto openDirect(std::int32_t directoryFileDescriptor, std::string_view path, open_how &openHow,
                        std::uint32_t fileDescriptorIndex) const noexcept -> void;

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

        auto adviseFile(std::int32_t fileDescriptor, std::uint64_t offset, off_t length,
                        std::int32_t advice) const noexcept -> void;

        auto truncate(std::int32_t fileDescriptor, loff_t length) const noexcept -> void;

        auto allocateFile(std::int32_t fileDescriptor, std::int32_t mode, std::uint64_t offset,
                          std::uint64_t length) const noexcept -> void;

        auto status(std::int32_t directoryFileDescriptor, std::string_view path, std::int32_t flags, std::uint32_t mask,
                    struct statx &buffer) const noexcept -> void;

        auto getExtendedAttribute(std::string_view path, std::string_view name, std::span<char> value) const noexcept
            -> void;

        auto getExtendedAttribute(std::int32_t fileDescriptor, std::string_view name,
                                  std::span<char> value) const noexcept -> void;

        auto setExtendedAttribute(std::string_view path, std::string_view name, std::span<char> value,
                                  std::int32_t flags) const noexcept -> void;

        auto setExtendedAttribute(std::int32_t fileDescriptor, std::string_view name, std::span<char> value,
                                  std::int32_t flags) const noexcept -> void;

        auto makeDirectory(std::string_view path, mode_t mode) const noexcept -> void;

        auto makeDirectory(std::int32_t directoryFileDescriptor, std::string_view path, mode_t mode) const noexcept
            -> void;

        auto rename(std::string_view oldPath, std::string_view newPath) const noexcept -> void;

        auto rename(std::int32_t oldDirectoryFileDescriptor, std::string_view oldPath,
                    std::int32_t newDirectoryFileDescriptor, std::string_view newPath,
                    std::uint32_t flags) const noexcept -> void;

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

        auto adviseMemory(std::span<std::byte> buffer, std::int32_t advice) const noexcept -> void;

        auto wait(idtype_t idType, id_t id, siginfo_t *signalInformation, std::int32_t options,
                  std::uint32_t flags) const noexcept -> void;

        auto waitFutex(std::uint32_t &futex, std::uint64_t value, std::uint64_t mask, std::uint32_t futexFlags,
                       std::uint32_t flags) const noexcept -> void;

        auto waitFutex(std::span<futex_waitv> vectorizedFutexs, std::uint32_t flags) const noexcept -> void;

        auto wakeFutex(std::uint32_t &futex, std::uint64_t value, std::uint64_t mask, std::uint32_t futexFlags,
                       std::uint32_t flags) const noexcept -> void;

    private:
        io_uring_sqe *handle;
    };

    [[nodiscard]] auto operator==(Submission, Submission) noexcept -> bool;
}    // namespace coContext::internal

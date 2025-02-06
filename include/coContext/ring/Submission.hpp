#pragma once

#include <filesystem>
#include <liburing.h>

namespace coContext::internal {
    class Submission {
    public:
        [[nodiscard]] static auto linkTimeout(io_uring_sqe *handle, __kernel_timespec *timeSpecification,
                                              std::uint32_t flags) noexcept -> Submission;

        [[nodiscard]] static auto noOperation(io_uring_sqe *handle) noexcept -> Submission;

        [[nodiscard]] static auto cancel(io_uring_sqe *handle, std::uint64_t userData, std::int32_t flags) noexcept
            -> Submission;

        [[nodiscard]] static auto cancel(io_uring_sqe *handle, std::int32_t fileDescriptor,
                                         std::uint32_t flags) noexcept -> Submission;

        [[nodiscard]] static auto timeout(io_uring_sqe *handle, __kernel_timespec *timeSpecification,
                                          std::uint32_t count, std::uint32_t flags) noexcept -> Submission;

        [[nodiscard]] static auto updateTimeout(io_uring_sqe *handle, std::uint64_t userData,
                                                __kernel_timespec *timeSpecification, std::uint32_t flags) noexcept
            -> Submission;

        [[nodiscard]] static auto poll(io_uring_sqe *handle, std::int32_t fileDescriptor, std::uint32_t mask) noexcept
            -> Submission;

        [[nodiscard]] static auto updatePoll(io_uring_sqe *handle, std::uint64_t oldUserData, std::uint64_t newUserData,
                                             std::uint32_t mask, std::uint32_t flags) noexcept -> Submission;

        [[nodiscard]] static auto multiplePoll(io_uring_sqe *handle, std::int32_t fileDescriptor,
                                               std::uint32_t mask) noexcept -> Submission;

        [[nodiscard]] static auto updateFileDescriptors(io_uring_sqe *handle, std::span<std::int32_t> fileDescriptors,
                                                        std::int32_t offset) noexcept -> Submission;

        [[nodiscard]] static auto installDirect(io_uring_sqe *handle, std::int32_t directFileDescriptor,
                                                std::uint32_t flags) noexcept -> Submission;

        [[nodiscard]] static auto close(io_uring_sqe *handle, std::int32_t fileDescriptor) noexcept -> Submission;

        [[nodiscard]] static auto closeDirect(io_uring_sqe *handle, std::int32_t directFileDescriptor) noexcept
            -> Submission;

        [[nodiscard]] static auto shutdown(io_uring_sqe *handle, std::int32_t socketFileDescriptor,
                                           std::int32_t how) noexcept -> Submission;

        [[nodiscard]] static auto socket(io_uring_sqe *handle, std::int32_t domain, std::int32_t type,
                                         std::int32_t protocol, std::uint32_t flags) noexcept -> Submission;

        [[nodiscard]] static auto directSocket(io_uring_sqe *handle, std::int32_t domain, std::int32_t type,
                                               std::int32_t protocol, std::uint32_t flags) noexcept -> Submission;

        [[nodiscard]] static auto socketCommand(io_uring_sqe *handle, std::int32_t operation,
                                                std::int32_t socketFileDescriptor, std::int32_t level,
                                                std::int32_t optionName, std::span<std::byte> option) noexcept
            -> Submission;

        [[nodiscard]] static auto discardCommand(io_uring_sqe *handle, std::int32_t fileDescriptor,
                                                 std::uint64_t offset, std::uint64_t length) noexcept -> Submission;

        [[nodiscard]] static auto bind(io_uring_sqe *handle, std::int32_t socketFileDescriptor, sockaddr *address,
                                       socklen_t addressLength) noexcept -> Submission;

        [[nodiscard]] static auto listen(io_uring_sqe *handle, std::int32_t socketFileDescriptor,
                                         std::int32_t backlog) noexcept -> Submission;

        [[nodiscard]] static auto accept(io_uring_sqe *handle, std::int32_t socketFileDescriptor, sockaddr *address,
                                         socklen_t *addressLength, std::int32_t flags) noexcept -> Submission;

        [[nodiscard]] static auto acceptDirect(io_uring_sqe *handle, std::int32_t socketFileDescriptor,
                                               sockaddr *address, socklen_t *addressLength, std::int32_t flags,
                                               std::uint32_t fileDescriptorIndex) noexcept -> Submission;

        [[nodiscard]] static auto multipleAccept(io_uring_sqe *handle, std::int32_t socketFileDescriptor,
                                                 sockaddr *address, socklen_t *addressLength,
                                                 std::int32_t flags) noexcept -> Submission;

        [[nodiscard]] static auto multipleAcceptDirect(io_uring_sqe *handle, std::int32_t socketFileDescriptor,
                                                       sockaddr *address, socklen_t *addressLength,
                                                       std::int32_t flags) noexcept -> Submission;

        [[nodiscard]] static auto connect(io_uring_sqe *handle, std::int32_t socketFileDescriptor,
                                          const sockaddr &address, socklen_t addressLength) noexcept -> Submission;

        [[nodiscard]] static auto receive(io_uring_sqe *handle, std::int32_t socketFileDescriptor,
                                          std::span<std::byte> buffer, std::int32_t flags) noexcept -> Submission;

        [[nodiscard]] static auto receive(io_uring_sqe *handle, std::int32_t socketFileDescriptor, msghdr &message,
                                          std::uint32_t flags) noexcept -> Submission;

        [[nodiscard]] static auto multipleReceive(io_uring_sqe *handle, std::int32_t socketFileDescriptor,
                                                  std::span<std::byte> buffer, std::int32_t flags) noexcept
            -> Submission;

        [[nodiscard]] static auto send(io_uring_sqe *handle, std::int32_t socketFileDescriptor,
                                       std::span<const std::byte> buffer, std::int32_t flags) noexcept -> Submission;

        [[nodiscard]] static auto send(io_uring_sqe *handle, std::int32_t socketFileDescriptor,
                                       std::span<const std::byte> buffer, std::int32_t flags,
                                       const sockaddr &destinationAddress, socklen_t destinationAddressLength) noexcept
            -> Submission;

        [[nodiscard]] static auto send(io_uring_sqe *handle, std::int32_t socketFileDescriptor, const msghdr &message,
                                       std::uint32_t flags) noexcept -> Submission;

        [[nodiscard]] static auto zeroCopySend(io_uring_sqe *handle, std::int32_t socketFileDescriptor,
                                               std::span<const std::byte> buffer, std::int32_t flags,
                                               std::uint32_t zeroCopyFlags) noexcept -> Submission;

        [[nodiscard]] static auto zeroCopySend(io_uring_sqe *handle, std::int32_t socketFileDescriptor,
                                               const msghdr &message, std::uint32_t flags) noexcept -> Submission;

        [[nodiscard]] static auto splice(io_uring_sqe *handle, std::int32_t inFileDescriptor, std::int64_t inOffset,
                                         std::int32_t outFileDescriptor, std::int64_t outOffset, std::uint32_t length,
                                         std::uint32_t flags) noexcept -> Submission;

        [[nodiscard]] static auto tee(io_uring_sqe *handle, std::int32_t inFileDescriptor,
                                      std::int32_t outFileDescriptor, std::uint32_t length,
                                      std::uint32_t flags) noexcept -> Submission;

        [[nodiscard]] static auto open(io_uring_sqe *handle, const std::filesystem::path &path, std::int32_t flags,
                                       mode_t mode) noexcept -> Submission;

        [[nodiscard]] static auto open(io_uring_sqe *handle, std::int32_t directoryFileDescriptor,
                                       const std::filesystem::path &path, std::int32_t flags, mode_t mode) noexcept
            -> Submission;

        [[nodiscard]] static auto open(io_uring_sqe *handle, std::int32_t directoryFileDescriptor,
                                       const std::filesystem::path &path, open_how &openHow) noexcept -> Submission;

        [[nodiscard]] static auto openDirect(io_uring_sqe *handle, const std::filesystem::path &path,
                                             std::int32_t flags, mode_t mode,
                                             std::uint32_t fileDescriptorIndex) noexcept -> Submission;

        [[nodiscard]] static auto openDirect(io_uring_sqe *handle, std::int32_t directoryFileDescriptor,
                                             const std::filesystem::path &path, std::int32_t flags, mode_t mode,
                                             std::uint32_t fileDescriptorIndex) noexcept -> Submission;

        [[nodiscard]] static auto openDirect(io_uring_sqe *handle, std::int32_t directoryFileDescriptor,
                                             const std::filesystem::path &path, open_how &openHow,
                                             std::uint32_t fileDescriptorIndex) noexcept -> Submission;

        [[nodiscard]] static auto read(io_uring_sqe *handle, std::int32_t fileDescriptor, std::span<std::byte> buffer,
                                       std::uint64_t offset) noexcept -> Submission;

        [[nodiscard]] static auto read(io_uring_sqe *handle, std::int32_t fileDescriptor, std::span<const iovec> buffer,
                                       std::uint64_t offset) noexcept -> Submission;

        [[nodiscard]] static auto read(io_uring_sqe *handle, std::int32_t fileDescriptor, std::span<const iovec> buffer,
                                       std::uint64_t offset, std::int32_t flags) noexcept -> Submission;

        [[nodiscard]] static auto multipleRead(io_uring_sqe *handle, std::int32_t fileDescriptor, std::uint32_t length,
                                               std::uint64_t offset, std::int32_t bufferGroup) noexcept -> Submission;

        [[nodiscard]] static auto write(io_uring_sqe *handle, std::int32_t fileDescriptor,
                                        std::span<const std::byte> buffer, std::uint64_t offset) noexcept -> Submission;

        [[nodiscard]] static auto write(io_uring_sqe *handle, std::int32_t fileDescriptor,
                                        std::span<const iovec> buffer, std::uint64_t offset) noexcept -> Submission;

        [[nodiscard]] static auto write(io_uring_sqe *handle, std::int32_t fileDescriptor,
                                        std::span<const iovec> buffer, std::uint64_t offset,
                                        std::int32_t flags) noexcept -> Submission;

        [[nodiscard]] static auto syncFile(io_uring_sqe *handle, std::int32_t fileDescriptor,
                                           std::uint32_t flags) noexcept -> Submission;

        [[nodiscard]] static auto syncFile(io_uring_sqe *handle, std::int32_t fileDescriptor, std::uint64_t offset,
                                           std::uint32_t length, std::int32_t flags) noexcept -> Submission;

        [[nodiscard]] static auto adviseFile(io_uring_sqe *handle, std::int32_t fileDescriptor, std::uint64_t offset,
                                             off_t length, std::int32_t advice) noexcept -> Submission;

        [[nodiscard]] static auto truncate(io_uring_sqe *handle, std::int32_t fileDescriptor, loff_t length) noexcept
            -> Submission;

        [[nodiscard]] static auto allocateFile(io_uring_sqe *handle, std::int32_t fileDescriptor, std::int32_t mode,
                                               std::uint64_t offset, std::uint64_t length) noexcept -> Submission;

        [[nodiscard]] static auto status(io_uring_sqe *handle, std::int32_t directoryFileDescriptor,
                                         const std::filesystem::path &path, std::int32_t flags, std::uint32_t mask,
                                         struct statx &buffer) noexcept -> Submission;

        [[nodiscard]] static auto getExtendedAttribute(io_uring_sqe *handle, const std::filesystem::path &path,
                                                       std::string_view name, std::span<char> value) noexcept
            -> Submission;

        [[nodiscard]] static auto getExtendedAttribute(io_uring_sqe *handle, std::int32_t fileDescriptor,
                                                       std::string_view name, std::span<char> value) noexcept
            -> Submission;

        [[nodiscard]] static auto setExtendedAttribute(io_uring_sqe *handle, const std::filesystem::path &path,
                                                       std::string_view name, std::span<char> value,
                                                       std::int32_t flags) noexcept -> Submission;

        [[nodiscard]] static auto setExtendedAttribute(io_uring_sqe *handle, std::int32_t fileDescriptor,
                                                       std::string_view name, std::span<char> value,
                                                       std::int32_t flags) noexcept -> Submission;

        [[nodiscard]] static auto makeDirectory(io_uring_sqe *handle, const std::filesystem::path &path,
                                                mode_t mode) noexcept -> Submission;

        [[nodiscard]] static auto makeDirectory(io_uring_sqe *handle, std::int32_t directoryFileDescriptor,
                                                const std::filesystem::path &path, mode_t mode) noexcept -> Submission;

        [[nodiscard]] static auto rename(io_uring_sqe *handle, const std::filesystem::path &oldPath,
                                         const std::filesystem::path &newPath) noexcept -> Submission;

        [[nodiscard]] static auto rename(io_uring_sqe *handle, std::int32_t oldDirectoryFileDescriptor,
                                         const std::filesystem::path &oldPath, std::int32_t newDirectoryFileDescriptor,
                                         const std::filesystem::path &newPath, std::uint32_t flags) noexcept
            -> Submission;

        [[nodiscard]] static auto link(io_uring_sqe *handle, const std::filesystem::path &oldPath,
                                       const std::filesystem::path &newPath, std::int32_t flags) noexcept -> Submission;

        [[nodiscard]] static auto link(io_uring_sqe *handle, std::int32_t oldDirectoryFileDescriptor,
                                       const std::filesystem::path &oldPath, std::int32_t newDirectoryFileDescriptor,
                                       const std::filesystem::path &newPath, std::int32_t flags) noexcept -> Submission;

        [[nodiscard]] static auto symbolicLink(io_uring_sqe *handle, std::string_view target,
                                               const std::filesystem::path &linkPath) noexcept -> Submission;

        [[nodiscard]] static auto symbolicLink(io_uring_sqe *handle, std::string_view target,
                                               std::int32_t newDirectoryFileDescriptor,
                                               const std::filesystem::path &linkPath) noexcept -> Submission;

        [[nodiscard]] static auto unlink(io_uring_sqe *handle, const std::filesystem::path &path,
                                         std::int32_t flags) noexcept -> Submission;

        [[nodiscard]] static auto unlink(io_uring_sqe *handle, std::int32_t directoryFileDescriptor,
                                         const std::filesystem::path &path, std::int32_t flags) noexcept -> Submission;

        [[nodiscard]] static auto adviseMemory(io_uring_sqe *handle, std::span<std::byte> buffer,
                                               std::int32_t advice) noexcept -> Submission;

        [[nodiscard]] static auto wait(io_uring_sqe *handle, idtype_t idType, id_t id, siginfo_t *signalInformation,
                                       std::int32_t options, std::uint32_t flags) noexcept -> Submission;

        [[nodiscard]] static auto waitFutex(io_uring_sqe *handle, std::uint32_t &futex, std::uint64_t value,
                                            std::uint64_t mask, std::uint32_t futexFlags, std::uint32_t flags) noexcept
            -> Submission;

        [[nodiscard]] static auto waitFutex(io_uring_sqe *handle, std::span<futex_waitv> vectorizedFutexs,
                                            std::uint32_t flags) noexcept -> Submission;

        [[nodiscard]] static auto wakeFutex(io_uring_sqe *handle, std::uint32_t &futex, std::uint64_t value,
                                            std::uint64_t mask, std::uint32_t futexFlags, std::uint32_t flags) noexcept
            -> Submission;

        explicit Submission(io_uring_sqe *handle = {}) noexcept;

        [[nodiscard]] auto get() const noexcept -> io_uring_sqe *;

        auto addFlags(std::uint32_t flags) const noexcept -> void;

        auto addIoPriority(std::uint16_t ioPriority) const noexcept -> void;

        auto setUserData(std::uint64_t userData) const noexcept -> void;

        auto setBufferGroup(std::uint16_t bufferGroup) const noexcept -> void;

    private:
        io_uring_sqe *handle;
    };

    [[nodiscard]] auto operator==(Submission, Submission) noexcept -> bool;
}    // namespace coContext::internal

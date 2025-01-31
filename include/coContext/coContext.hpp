#pragma once

#include "coroutine/AsyncWaiter.hpp"
#include "coroutine/Marker.hpp"
#include "coroutine/Task.hpp"
#include "log/logger.hpp"

namespace coContext {
    template<internal::Returnable T = void>
    struct SpawnResult {
        std::future<T> value;
        std::uint64_t taskId;
    };

    template<typename T>
    struct SpawnResult<T &> {
        std::future<T &> value;
        std::uint64_t taskId;
    };

    template<>
    struct SpawnResult<> {
        std::future<void> value;
        std::uint64_t taskId;
    };

    enum class ClockSource : std::uint8_t { monotonic, absolute, boot, real };

    namespace internal {
        auto spawn(Coroutine coroutine) -> void;
    }    // namespace internal

    auto run() -> void;

    auto stop() -> void;

    template<std::movable T, typename F, typename... Args>
        requires std::is_invocable_r_v<Task<T>, F, Args...>
    constexpr auto spawn(F &&f, Args &&...args) {
        Task<T> task{std::invoke(std::forward<F>(f), std::forward<Args>(args)...)};

        internal::Coroutine coroutine{std::move(task.getCoroutine())};
        const std::uint64_t id{std::hash<internal::Coroutine>{}(coroutine)};

        spawn(std::move(coroutine));

        return SpawnResult{std::move(task.getReturnValue()), id};
    }

    template<typename T, typename F, typename... Args>
        requires std::is_lvalue_reference_v<T> && std::is_invocable_r_v<Task<T &>, F, Args...>
    constexpr auto spawn(F &&f, Args &&...args) {
        Task<T &> task{std::invoke(std::forward<F>(f), std::forward<Args>(args)...)};

        internal::Coroutine coroutine{std::move(task.getCoroutine())};
        const std::uint64_t id{std::hash<internal::Coroutine>{}(coroutine)};

        spawn(std::move(coroutine));

        return SpawnResult{std::move(task.getReturnValue()), id};
    }

    template<typename F, typename... Args>
        requires std::is_invocable_r_v<Task<>, F, Args...>
    constexpr auto spawn(F &&f, Args &&...args) {
        Task<> task{std::invoke(std::forward<F>(f), std::forward<Args>(args)...)};

        internal::Coroutine coroutine{std::move(task.getCoroutine())};
        const std::uint64_t id{std::hash<internal::Coroutine>{}(coroutine)};

        spawn(std::move(coroutine));

        return SpawnResult{std::move(task.getReturnValue()), id};
    }

    [[nodiscard]] auto syncCancel(std::uint64_t taskId, std::chrono::seconds seconds = {},
                                  std::chrono::nanoseconds nanoseconds = {}) -> std::int32_t;

    [[nodiscard]] auto syncCancel(std::int32_t fileDescriptor, bool isMatchAll = {}, std::chrono::seconds seconds = {},
                                  std::chrono::nanoseconds nanoseconds = {}) -> std::int32_t;

    [[nodiscard]] auto syncCancelAny(std::chrono::seconds seconds = {}, std::chrono::nanoseconds nanoseconds = {})
        -> std::int32_t;

    [[nodiscard]] auto toDirect(std::span<std::int32_t> fileDescriptors) -> internal::AsyncWaiter;

    [[nodiscard]] auto installDirect(std::int32_t directFileDescriptor, bool isCloseOnExecute = true)
        -> internal::AsyncWaiter;

    [[nodiscard]] auto none() noexcept -> internal::Marker;

    [[nodiscard]] auto direct() noexcept -> internal::Marker;

    [[nodiscard]] auto timeout(std::chrono::seconds seconds, std::chrono::nanoseconds nanoseconds = {},
                               ClockSource clockSource = {}) -> internal::Marker;

    [[nodiscard]] auto cancel(std::uint64_t taskId) -> internal::AsyncWaiter;

    [[nodiscard]] auto cancel(std::int32_t fileDescriptor, bool isMatchAll = {}) -> internal::AsyncWaiter;

    [[nodiscard]] auto cancelAny() -> internal::AsyncWaiter;

    [[nodiscard]] auto sleep(std::chrono::seconds seconds, std::chrono::nanoseconds nanoseconds = {},
                             ClockSource clockSource = {}) -> internal::AsyncWaiter;

    [[nodiscard]] auto updateSleep(std::uint64_t taskId, std::chrono::seconds seconds,
                                   std::chrono::nanoseconds nanoseconds = {}, ClockSource clockSource = {})
        -> internal::AsyncWaiter;

    [[nodiscard]] auto multipleSleep(std::move_only_function<auto(std::int32_t)->void> action,
                                     std::chrono::seconds seconds, std::chrono::nanoseconds nanoseconds = {},
                                     ClockSource clockSource = {}, internal::Marker marker = none()) -> Task<>;

    [[nodiscard]] auto poll(std::int32_t fileDescriptor, std::uint32_t mask) -> internal::AsyncWaiter;

    [[nodiscard]] auto updatePoll(std::uint64_t taskId, std::uint32_t mask) -> internal::AsyncWaiter;

    [[nodiscard]] auto multiplePoll(std::move_only_function<auto(std::int32_t)->void> action,
                                    std::int32_t fileDescriptor, std::uint32_t mask, internal::Marker marker = none())
        -> Task<>;

    [[nodiscard]] auto close(std::int32_t fileDescriptor) -> internal::AsyncWaiter;

    [[nodiscard]] auto closeDirect(std::int32_t directFileDescriptor) -> internal::AsyncWaiter;

    [[nodiscard]] auto socket(std::int32_t domain, std::int32_t type, std::int32_t protocol) -> internal::AsyncWaiter;

    [[nodiscard]] auto directSocket(std::int32_t domain, std::int32_t type, std::int32_t protocol)
        -> internal::AsyncWaiter;

    [[nodiscard]] auto getSocketOption(std::int32_t socketFileDescriptor, std::int32_t level, std::int32_t optionName,
                                       std::span<std::byte> option) -> internal::AsyncWaiter;

    [[nodiscard]] auto setSocketOption(std::int32_t socketFileDescriptor, std::int32_t level, std::int32_t optionName,
                                       std::span<std::byte> option) -> internal::AsyncWaiter;

    [[nodiscard]] auto getSocketReceiveBufferUnreadDataSize(std::int32_t socketFileDescriptor) -> internal::AsyncWaiter;

    [[nodiscard]] auto getSocketSendBufferUnsentDataSize(std::int32_t socketFileDescriptor) -> internal::AsyncWaiter;

    [[nodiscard]] auto discardData(std::int32_t fileDescriptor, std::uint64_t offset, std::uint64_t length)
        -> internal::AsyncWaiter;

    [[nodiscard]] auto bind(std::int32_t socketFileDescriptor, sockaddr &address, socklen_t addressLength)
        -> internal::AsyncWaiter;

    [[nodiscard]] auto listen(std::int32_t socketFileDescriptor, std::int32_t backlog) -> internal::AsyncWaiter;

    [[nodiscard]] auto accept(std::int32_t socketFileDescriptor, sockaddr *address, socklen_t *addressLength,
                              std::int32_t flags = {}) -> internal::AsyncWaiter;

    [[nodiscard]] auto acceptDirect(std::int32_t socketFileDescriptor, sockaddr *address, socklen_t *addressLength,
                                    std::int32_t flags = {}) -> internal::AsyncWaiter;

    [[nodiscard]] auto multipleAccept(std::move_only_function<auto(std::int32_t)->void> action,
                                      std::int32_t socketFileDescriptor, sockaddr *address, socklen_t *addressLength,
                                      std::int32_t flags = {}, internal::Marker marker = none()) -> Task<>;

    [[nodiscard]] auto multipleAcceptDirect(std::move_only_function<auto(std::int32_t)->void> action,
                                            std::int32_t socketFileDescriptor, sockaddr *address,
                                            socklen_t *addressLength, std::int32_t flags = {},
                                            internal::Marker marker = none()) -> Task<>;

    [[nodiscard]] auto connect(std::int32_t socketFileDescriptor, sockaddr address, socklen_t addressLength)
        -> internal::AsyncWaiter;

    [[nodiscard]] auto shutdown(std::int32_t socketFileDescriptor, std::int32_t how) -> internal::AsyncWaiter;

    [[nodiscard]] auto receive(std::int32_t socketFileDescriptor, std::span<std::byte> buffer, std::int32_t flags)
        -> internal::AsyncWaiter;

    [[nodiscard]] auto receive(std::int32_t socketFileDescriptor, msghdr &message, std::uint32_t flags)
        -> internal::AsyncWaiter;

    [[nodiscard]] auto
        multipleReceive(std::move_only_function<auto(std::int32_t, std::span<const std::byte>)->void> action,
                        std::int32_t socketFileDescriptor, std::int32_t flags, internal::Marker marker = none())
            -> Task<>;

    [[nodiscard]] auto send(std::int32_t socketFileDescriptor, std::span<const std::byte> buffer, std::int32_t flags)
        -> internal::AsyncWaiter;

    [[nodiscard]] auto send(std::int32_t socketFileDescriptor, std::span<const std::byte> buffer, std::int32_t flags,
                            sockaddr destinationAddress, socklen_t destinationAddressLength) -> internal::AsyncWaiter;

    [[nodiscard]] auto send(std::int32_t socketFileDescriptor, const msghdr &message, std::uint32_t flags)
        -> internal::AsyncWaiter;

    [[nodiscard]] auto zeroCopySend(std::move_only_function<auto(std::int32_t)->void> action,
                                    std::int32_t socketFileDescriptor, std::span<const std::byte> buffer,
                                    std::int32_t flags, internal::Marker marker = none()) -> Task<>;

    [[nodiscard]] auto zeroCopySend(std::move_only_function<auto(std::int32_t)->void> action,
                                    std::int32_t socketFileDescriptor, const msghdr &message, std::int32_t flags,
                                    internal::Marker marker = none()) -> Task<>;

    [[nodiscard]] auto splice(std::int32_t inFileDescriptor, std::int64_t inOffset, std::int32_t outFileDescriptor,
                              std::int64_t outOffset, std::uint32_t length, std::uint32_t flags)
        -> internal::AsyncWaiter;

    [[nodiscard]] auto tee(std::int32_t inFileDescriptor, std::int32_t outFileDescriptor, std::uint32_t length,
                           std::uint32_t flags) -> internal::AsyncWaiter;

    [[nodiscard]] auto open(const std::filesystem::path &path, std::int32_t flags, mode_t mode = {})
        -> internal::AsyncWaiter;

    [[nodiscard]] auto open(std::int32_t directoryFileDescriptor, const std::filesystem::path &path, std::int32_t flags,
                            mode_t mode = {}) -> internal::AsyncWaiter;

    [[nodiscard]] auto open(std::int32_t directoryFileDescriptor, const std::filesystem::path &path, open_how &openHow)
        -> internal::AsyncWaiter;

    [[nodiscard]] auto openDirect(const std::filesystem::path &path, std::int32_t flags, mode_t mode = {})
        -> internal::AsyncWaiter;

    [[nodiscard]] auto openDirect(std::int32_t directoryFileDescriptor, const std::filesystem::path &path,
                                  std::int32_t flags, mode_t mode = {}) -> internal::AsyncWaiter;

    [[nodiscard]] auto openDirect(std::int32_t directoryFileDescriptor, const std::filesystem::path &path,
                                  open_how &openHow) -> internal::AsyncWaiter;

    [[nodiscard]] auto read(std::int32_t fileDescriptor, std::span<std::byte> buffer, std::uint64_t offset = -1)
        -> internal::AsyncWaiter;

    [[nodiscard]] auto read(std::int32_t fileDescriptor, std::span<const iovec> buffer, std::uint64_t offset = -1)
        -> internal::AsyncWaiter;

    [[nodiscard]] auto read(std::int32_t fileDescriptor, std::span<const iovec> buffer, std::uint64_t offset,
                            std::int32_t flags) -> internal::AsyncWaiter;

    [[nodiscard]] auto
        multipleRead(std::move_only_function<auto(std::int32_t, std::span<const std::byte>)->void> action,
                     std::int32_t fileDescriptor, std::int32_t offset = -1, internal::Marker marker = none()) -> Task<>;

    [[nodiscard]] auto write(std::int32_t fileDescriptor, std::span<const std::byte> buffer, std::uint64_t offset = -1)
        -> internal::AsyncWaiter;

    [[nodiscard]] auto write(std::int32_t fileDescriptor, std::span<const iovec> buffer, std::uint64_t offset = -1)
        -> internal::AsyncWaiter;

    [[nodiscard]] auto write(std::int32_t fileDescriptor, std::span<const iovec> buffer, std::uint64_t offset,
                             std::int32_t flags) -> internal::AsyncWaiter;

    [[nodiscard]] auto syncFile(std::int32_t fileDescriptor, bool isSyncMetadata = true) -> internal::AsyncWaiter;

    [[nodiscard]] auto syncFile(std::int32_t fileDescriptor, std::uint64_t offset, std::uint32_t length,
                                std::int32_t flags) -> internal::AsyncWaiter;

    [[nodiscard]] auto adviseFile(std::int32_t fileDescriptor, std::uint64_t offset, off_t length, std::int32_t advice)
        -> internal::AsyncWaiter;

    [[nodiscard]] auto truncate(std::int32_t fileDescriptor, loff_t length) -> internal::AsyncWaiter;

    [[nodiscard]] auto allocateFile(std::int32_t fileDescriptor, std::int32_t mode, std::uint64_t offset,
                                    std::uint64_t length) -> internal::AsyncWaiter;

    [[nodiscard]] auto status(std::int32_t directoryFileDescriptor, const std::filesystem::path &path,
                              std::int32_t flags, std::uint32_t mask, struct statx &buffer) -> internal::AsyncWaiter;

    [[nodiscard]] auto getExtendedAttribute(const std::filesystem::path &path, std::string_view name,
                                            std::span<char> value) -> internal::AsyncWaiter;

    [[nodiscard]] auto getExtendedAttribute(std::int32_t fileDescriptor, std::string_view name, std::span<char> value)
        -> internal::AsyncWaiter;

    [[nodiscard]] auto setExtendedAttribute(const std::filesystem::path &path, std::string_view name,
                                            std::span<char> value, std::int32_t flags) -> internal::AsyncWaiter;

    [[nodiscard]] auto setExtendedAttribute(std::int32_t fileDescriptor, std::string_view name, std::span<char> value,
                                            std::int32_t flags) -> internal::AsyncWaiter;

    [[nodiscard]] auto makeDirectory(const std::filesystem::path &path, mode_t mode) -> internal::AsyncWaiter;

    [[nodiscard]] auto makeDirectory(std::int32_t directoryFileDescriptor, const std::filesystem::path &path,
                                     mode_t mode) -> internal::AsyncWaiter;

    [[nodiscard]] auto rename(const std::filesystem::path &oldPath, const std::filesystem::path &newPath)
        -> internal::AsyncWaiter;

    [[nodiscard]] auto rename(std::int32_t oldDirectoryFileDescriptor, const std::filesystem::path &oldPath,
                              std::int32_t newDirectoryFileDescriptor, const std::filesystem::path &newPath,
                              std::uint32_t flags = {}) -> internal::AsyncWaiter;

    [[nodiscard]] auto link(const std::filesystem::path &oldPath, const std::filesystem::path &newPath)
        -> internal::AsyncWaiter;

    [[nodiscard]] auto link(std::int32_t oldDirectoryFileDescriptor, const std::filesystem::path &oldPath,
                            std::int32_t newDirectoryFileDescriptor, const std::filesystem::path &newPath,
                            std::int32_t flags) -> internal::AsyncWaiter;

    [[nodiscard]] auto symbolicLink(std::string_view target, const std::filesystem::path &linkPath)
        -> internal::AsyncWaiter;

    [[nodiscard]] auto symbolicLink(std::string_view target, std::int32_t newDirectoryFileDescriptor,
                                    const std::filesystem::path &linkPath) -> internal::AsyncWaiter;

    [[nodiscard]] auto unlink(const std::filesystem::path &path) -> internal::AsyncWaiter;

    [[nodiscard]] auto unlink(std::int32_t directoryFileDescriptor, const std::filesystem::path &path,
                              std::int32_t flags) -> internal::AsyncWaiter;

    [[nodiscard]] auto adviseMemory(std::span<std::byte> buffer, std::int32_t advice) -> internal::AsyncWaiter;

    [[nodiscard]] auto wait(idtype_t idType, id_t id, siginfo_t *signalInformation, std::int32_t options)
        -> internal::AsyncWaiter;

    [[nodiscard]] auto waitFutex(std::uint32_t &futex, std::uint64_t value, std::uint64_t mask, std::uint32_t flags)
        -> internal::AsyncWaiter;

    [[nodiscard]] auto waitFutex(std::span<futex_waitv> vectorizedFutexs) -> internal::AsyncWaiter;

    [[nodiscard]] auto wakeFutex(std::uint32_t &futex, std::uint64_t value, std::uint64_t mask, std::uint32_t flags)
        -> internal::AsyncWaiter;
}    // namespace coContext

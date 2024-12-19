#pragma once

#include "coroutine/AsyncWaiter.hpp"
#include "coroutine/Marker.hpp"
#include "coroutine/Task.hpp"

#include <functional>

namespace coContext {
    template<TaskReturnType T = void>
    struct SpawnResult {
        std::uint64_t taskIdentity;
        std::future<T> result;
    };

    template<typename T>
    struct SpawnResult<T &> {
        std::uint64_t taskIdentity;
        std::future<T &> result;
    };

    template<>
    struct SpawnResult<> {
        std::uint64_t taskIdentity;
        std::future<void> result;
    };

    enum class ClockSource : std::uint8_t { monotonic, absolute, boot, real };

    auto run() -> void;

    auto stop() noexcept -> void;

    auto spawn(Coroutine &&coroutine) -> void;

    template<std::movable T, typename F, typename... Args>
        requires std::is_invocable_r_v<Task<T>, F, Args...>
    constexpr auto spawn(F &&func, Args &&...args) {
        Task<T> task{std::invoke(func, std::forward<Args>(args)...)};

        Coroutine &coroutine{task.getCoroutine()};
        const std::uint64_t identity{std::hash<Coroutine>{}(coroutine)};

        spawn(std::move(coroutine));

        return SpawnResult{identity, std::move(task.getReturnValue())};
    }

    template<typename T, typename F, typename... Args>
        requires std::is_lvalue_reference_v<T> && std::is_invocable_r_v<Task<T &>, F, Args...>
    constexpr auto spawn(F &&func, Args &&...args) {
        Task<T &> task{std::invoke(func, std::forward<Args>(args)...)};

        Coroutine &coroutine{task.getCoroutine()};
        const std::uint64_t identity{std::hash<Coroutine>{}(coroutine)};

        spawn(std::move(coroutine));

        return SpawnResult{identity, std::move(task.getReturnValue())};
    }

    template<typename F, typename... Args>
        requires std::is_invocable_r_v<Task<>, F, Args...>
    constexpr auto spawn(F &&func, Args &&...args) {
        Task<> task{std::invoke(func, std::forward<Args>(args)...)};

        Coroutine &coroutine{task.getCoroutine()};
        const std::uint64_t identity{std::hash<Coroutine>{}(coroutine)};

        spawn(std::move(coroutine));

        return SpawnResult{identity, std::move(task.getReturnValue())};
    }

    [[nodiscard]] auto syncCancel(std::uint64_t taskIdentity, std::chrono::seconds seconds = {},
                                  std::chrono::nanoseconds nanoseconds = {}) -> std::int32_t;

    [[nodiscard]] auto syncCancel(std::int32_t fileDescriptor, bool isMatchAll = {}, std::chrono::seconds seconds = {},
                                  std::chrono::nanoseconds nanoseconds = {}) -> std::int32_t;

    [[nodiscard]] auto syncCancelAny(std::chrono::seconds seconds = {}, std::chrono::nanoseconds nanoseconds = {})
        -> std::int32_t;

    [[nodiscard]] auto useDirectFileDescriptor() noexcept -> Marker;

    [[nodiscard]] auto timeout(std::chrono::seconds seconds, std::chrono::nanoseconds nanoseconds = {},
                               ClockSource clockSource = {}) -> Marker;

    [[nodiscard]] auto cancel(std::uint64_t taskIdentity) -> AsyncWaiter;

    [[nodiscard]] auto cancel(std::int32_t fileDescriptor, bool isMatchAll = {}) -> AsyncWaiter;

    [[nodiscard]] auto cancelAny() -> AsyncWaiter;

    [[nodiscard]] auto sleep(std::chrono::seconds seconds, std::chrono::nanoseconds nanoseconds = {},
                             ClockSource clockSource = {}) -> AsyncWaiter;

    [[nodiscard]] auto updateSleep(std::uint64_t taskIdentity, std::chrono::seconds seconds,
                                   std::chrono::nanoseconds nanoseconds = {}, ClockSource clockSource = {})
        -> AsyncWaiter;

    [[nodiscard]] auto poll(std::int32_t fileDescriptor, std::uint32_t mask) -> AsyncWaiter;

    [[nodiscard]] auto updatePoll(std::uint64_t taskIdentity, std::uint32_t mask) -> AsyncWaiter;

    [[nodiscard]] auto close(std::int32_t fileDescriptor) -> AsyncWaiter;

    [[nodiscard]] auto socket(std::int32_t domain, std::int32_t type, std::int32_t protocol) -> AsyncWaiter;

    [[nodiscard]] auto getSocketOption(std::int32_t socketFileDescriptor, std::int32_t level, std::int32_t optionName,
                                       void *optionValue, std::int32_t optionLength) -> AsyncWaiter;

    [[nodiscard]] auto setSocketOption(std::int32_t socketFileDescriptor, std::int32_t level, std::int32_t optionName,
                                       void *optionValue, std::int32_t optionLength) -> AsyncWaiter;

    [[nodiscard]] auto getSocketReceiveBufferUnreadDataSize(std::int32_t socketFileDescriptor) -> AsyncWaiter;

    [[nodiscard]] auto getSocketSendBufferUnsentDataSize(std::int32_t socketFileDescriptor) -> AsyncWaiter;

    [[nodiscard]] auto discardData(std::int32_t fileDescriptor, std::uint64_t offset, std::uint64_t length)
        -> AsyncWaiter;

    [[nodiscard]] auto bind(std::int32_t socketFileDescriptor, sockaddr &address, socklen_t addressLength)
        -> AsyncWaiter;

    [[nodiscard]] auto listen(std::int32_t socketFileDescriptor, std::int32_t backlog) -> AsyncWaiter;

    [[nodiscard]] auto accept(std::int32_t socketFileDescriptor, sockaddr *address, socklen_t *addressLength,
                              std::int32_t flags = {}) -> AsyncWaiter;

    [[nodiscard]] auto connect(std::int32_t socketFileDescriptor, const sockaddr &address, socklen_t addressLength)
        -> AsyncWaiter;

    [[nodiscard]] auto shutdown(std::int32_t socketFileDescriptor, std::int32_t how) -> AsyncWaiter;

    [[nodiscard]] auto receive(std::int32_t socketFileDescriptor, std::span<std::byte> buffer, std::int32_t flags)
        -> AsyncWaiter;

    [[nodiscard]] auto receive(std::int32_t socketFileDescriptor, msghdr &message, std::uint32_t flags) -> AsyncWaiter;

    [[nodiscard]] auto send(std::int32_t socketFileDescriptor, std::span<const std::byte> buffer, std::int32_t flags)
        -> AsyncWaiter;

    [[nodiscard]] auto send(std::int32_t socketFileDescriptor, std::span<const std::byte> buffer, std::int32_t flags,
                            const sockaddr &destinationAddress, socklen_t destinationAddressLength) -> AsyncWaiter;

    [[nodiscard]] auto send(std::int32_t socketFileDescriptor, const msghdr &message, std::uint32_t flags)
        -> AsyncWaiter;

    [[nodiscard]] auto splice(std::int32_t inFileDescriptor, std::int64_t inFileDescriptorOffset,
                              std::int32_t outFileDescriptor, std::int64_t outFileDescriptorOffset,
                              std::uint32_t length, std::uint32_t flags) -> AsyncWaiter;

    [[nodiscard]] auto tee(std::int32_t inFileDescriptor, std::int32_t outFileDescriptor, std::uint32_t length,
                           std::uint32_t flags) -> AsyncWaiter;

    [[nodiscard]] auto open(std::string_view path, std::int32_t flags, mode_t mode = {}) -> AsyncWaiter;

    [[nodiscard]] auto open(std::int32_t directoryFileDescriptor, std::string_view path, std::int32_t flags,
                            mode_t mode = {}) -> AsyncWaiter;

    [[nodiscard]] auto open(std::int32_t directoryFileDescriptor, std::string_view path, open_how &openHow)
        -> AsyncWaiter;

    [[nodiscard]] auto read(std::int32_t fileDescriptor, std::span<std::byte> buffer, std::uint64_t offset = -1)
        -> AsyncWaiter;

    [[nodiscard]] auto read(std::int32_t fileDescriptor, std::span<const iovec> buffer, std::uint64_t offset = -1)
        -> AsyncWaiter;

    [[nodiscard]] auto read(std::int32_t fileDescriptor, std::span<const iovec> buffer, std::uint64_t offset,
                            std::int32_t flags) -> AsyncWaiter;

    [[nodiscard]] auto write(std::int32_t fileDescriptor, std::span<const std::byte> buffer, std::uint64_t offset = -1)
        -> AsyncWaiter;

    [[nodiscard]] auto write(std::int32_t fileDescriptor, std::span<const iovec> buffer, std::uint64_t offset = -1)
        -> AsyncWaiter;

    [[nodiscard]] auto write(std::int32_t fileDescriptor, std::span<const iovec> buffer, std::uint64_t offset,
                             std::int32_t flags) -> AsyncWaiter;

    [[nodiscard]] auto syncFile(std::int32_t fileDescriptor, bool isSyncMetadata = true) -> AsyncWaiter;

    [[nodiscard]] auto syncFile(std::int32_t fileDescriptor, std::uint64_t offset, std::uint32_t length,
                                std::int32_t flags) -> AsyncWaiter;

    [[nodiscard]] auto adviseFile(std::int32_t fileDescriptor, std::uint64_t offset, off_t length, std::int32_t advice)
        -> AsyncWaiter;

    [[nodiscard]] auto truncate(std::int32_t fileDescriptor, loff_t length) -> AsyncWaiter;

    [[nodiscard]] auto allocateFile(std::int32_t fileDescriptor, std::int32_t mode, std::uint64_t offset,
                                    std::uint64_t length) -> AsyncWaiter;

    [[nodiscard]] auto getFileStatus(std::int32_t directoryFileDescriptor, std::string_view path, std::int32_t flags,
                                     std::uint32_t mask, struct statx &buffer) -> AsyncWaiter;

    [[nodiscard]] auto getExtendedAttribute(std::string_view path, std::string_view name, std::span<char> value)
        -> AsyncWaiter;

    [[nodiscard]] auto getExtendedAttribute(std::int32_t fileDescriptor, std::string_view name, std::span<char> value)
        -> AsyncWaiter;

    [[nodiscard]] auto setExtendedAttribute(std::string_view path, std::string_view name, std::span<char> value,
                                            std::int32_t flags) -> AsyncWaiter;

    [[nodiscard]] auto setExtendedAttribute(std::int32_t fileDescriptor, std::string_view name, std::span<char> value,
                                            std::int32_t flags) -> AsyncWaiter;

    [[nodiscard]] auto makeDirectory(std::string_view path, mode_t mode) -> AsyncWaiter;

    [[nodiscard]] auto makeDirectory(std::int32_t directoryFileDescriptor, std::string_view path, mode_t mode)
        -> AsyncWaiter;

    [[nodiscard]] auto rename(std::string_view oldPath, std::string_view newPath) -> AsyncWaiter;

    [[nodiscard]] auto rename(std::int32_t oldDirectoryFileDescriptor, std::string_view oldPath,
                              std::int32_t newDirectoryFileDescriptor, std::string_view newPath,
                              std::uint32_t flags = {}) -> AsyncWaiter;

    [[nodiscard]] auto link(std::string_view oldPath, std::string_view newPath) -> AsyncWaiter;

    [[nodiscard]] auto link(std::int32_t oldDirectoryFileDescriptor, std::string_view oldPath,
                            std::int32_t newDirectoryFileDescriptor, std::string_view newPath, std::int32_t flags)
        -> AsyncWaiter;

    [[nodiscard]] auto symbolicLink(std::string_view target, std::string_view linkPath) -> AsyncWaiter;

    [[nodiscard]] auto symbolicLink(std::string_view target, std::int32_t newDirectoryFileDescriptor,
                                    std::string_view linkPath) -> AsyncWaiter;

    [[nodiscard]] auto unlink(std::string_view path) -> AsyncWaiter;

    [[nodiscard]] auto unlink(std::int32_t directoryFileDescriptor, std::string_view path, std::int32_t flags)
        -> AsyncWaiter;

    [[nodiscard]] auto adviseMemory(std::span<std::byte> buffer, std::int32_t advice) -> AsyncWaiter;

    [[nodiscard]] auto wait(idtype_t idType, id_t id, siginfo_t *signalInformation, std::int32_t options)
        -> AsyncWaiter;

    [[nodiscard]] auto waitFutex(std::uint32_t &futex, std::uint64_t value, std::uint64_t mask, std::uint32_t flags)
        -> AsyncWaiter;

    [[nodiscard]] auto wakeFutex(std::uint32_t &futex, std::uint64_t value, std::uint64_t mask, std::uint32_t flags)
        -> AsyncWaiter;
}    // namespace coContext

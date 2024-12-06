#pragma once

#include "coroutine/AsyncWaiter.hpp"
#include "coroutine/GenericTask.hpp"
#include "coroutine/Task.hpp"

#include <functional>
#include <linux/openat2.h>
#include <sys/socket.h>

namespace coContext {
    template<TaskReturnType T>
    struct SpawnResult {
        std::future<T> result;
        std::uint64_t taskIdentity;
    };

    enum class ClockSource : std::uint8_t { monotonic, absolute, boot, real };

    auto spawn(GenericTask &&task) -> void;

    template<typename F, typename... Args>
        requires std::is_invocable_r_v<Task<>, F, Args...>
    constexpr auto spawn(F &&func, Args &&...args) {
        Task<> task{std::invoke(func, std::forward<Args>(args)...)};
        Coroutine &coroutine{task.getCoroutine()};

        const std::uint64_t taskIdentity{std::hash<Coroutine>{}(coroutine)};
        spawn(GenericTask{std::move(coroutine)});

        return SpawnResult{std::future<void>{}, taskIdentity};
    }

    template<TaskReturnType T, typename F, typename... Args>
        requires std::is_invocable_r_v<Task<T>, F, Args...>
    constexpr auto spawn(F &&func, Args &&...args) {
        Task<T> task{std::invoke(func, std::forward<Args>(args)...)};
        Coroutine &coroutine{task.getCoroutine()};

        const std::uint64_t taskIdentity{std::hash<Coroutine>{}(coroutine)};
        spawn(GenericTask{std::move(coroutine)});

        return SpawnResult{std::future<T>{std::move(task.getReturnValue())}, taskIdentity};
    }

    auto run() -> void;

    [[nodiscard]] auto stop() -> AsyncWaiter;

    [[nodiscard]] auto syncCancel(std::uint64_t taskIdentity, __kernel_timespec timeout = {}) -> std::int32_t;

    [[nodiscard]] auto syncCancel(std::int32_t fileDescriptor, bool isMatchAll = {}, __kernel_timespec timeout = {})
        -> std::int32_t;

    [[nodiscard]] auto syncCancelAny(__kernel_timespec timeout = {}) -> std::int32_t;

    [[nodiscard]] auto cancel(std::uint64_t taskIdentity) -> AsyncWaiter;

    [[nodiscard]] auto cancel(std::int32_t fileDescriptor, bool isMatchAll = {}) -> AsyncWaiter;

    [[nodiscard]] auto cancelAny() -> AsyncWaiter;

    [[nodiscard]] auto timeout(__kernel_timespec &timeout, ClockSource clockSource = {}) -> AsyncWaiter;

    [[nodiscard]] auto updateTimeout(__kernel_timespec &timeout, std::uint64_t taskIdentity,
                                     ClockSource clockSource = {}) -> AsyncWaiter;

    [[nodiscard]] auto removeTimeout(std::uint64_t taskIdentity) -> AsyncWaiter;

    [[nodiscard]] auto close(std::int32_t fileDescriptor) -> AsyncWaiter;

    [[nodiscard]] auto socket(std::int32_t domain, std::int32_t type, std::int32_t protocol) -> AsyncWaiter;

    [[nodiscard]] auto bind(std::int32_t socketFileDescriptor, sockaddr *address, std::uint32_t addressLength)
        -> AsyncWaiter;

    [[nodiscard]] auto listen(std::int32_t socketFileDescriptor, std::int32_t backlog) -> AsyncWaiter;

    [[nodiscard]] auto accept(std::int32_t socketFileDescriptor, sockaddr *address, std::uint32_t *addressLength,
                              std::int32_t flags = {}) -> AsyncWaiter;

    [[nodiscard]] auto connect(std::int32_t socketFileDescriptor, const sockaddr *address, std::uint32_t addressLength)
        -> AsyncWaiter;

    [[nodiscard]] auto shutdown(std::int32_t socketFileDescriptor, std::int32_t how) -> AsyncWaiter;

    [[nodiscard]] auto receive(std::int32_t socketFileDescriptor, std::span<std::byte> buffer, std::int32_t flags)
        -> AsyncWaiter;

    [[nodiscard]] auto receive(std::int32_t socketFileDescriptor, msghdr &message, std::uint32_t flags) -> AsyncWaiter;

    [[nodiscard]] auto send(std::int32_t socketFileDescriptor, std::span<const std::byte> buffer, std::int32_t flags)
        -> AsyncWaiter;

    [[nodiscard]] auto send(std::int32_t socketFileDescriptor, std::span<const std::byte> buffer, std::int32_t flags,
                            const sockaddr *address, std::uint32_t addressLength) -> AsyncWaiter;

    [[nodiscard]] auto send(std::int32_t socketFileDescriptor, const msghdr &message, std::uint32_t flags)
        -> AsyncWaiter;

    [[nodiscard]] auto open(std::string_view pathname, std::int32_t flags, std::uint32_t mode = {}) -> AsyncWaiter;

    [[nodiscard]] auto open(std::int32_t directoryFileDescriptor, std::string_view pathname, std::int32_t flags,
                            std::uint32_t mode = {}) -> AsyncWaiter;

    [[nodiscard]] auto open(std::int32_t directoryFileDescriptor, std::string_view pathname, open_how &how)
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
}    // namespace coContext

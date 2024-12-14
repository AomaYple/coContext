#pragma once

#include "coroutine/AsyncWaiter.hpp"
#include "coroutine/Task.hpp"

#include <functional>

namespace coContext {
    template<typename T>
        requires std::movable<T> || std::is_lvalue_reference_v<T>
    struct SpawnResult {
        std::uint64_t taskIdentity;
        std::future<T> result;
    };

    template<typename T>
    struct SpawnResult<T &> {
        std::uint64_t taskIdentity;
        std::future<T &> result;
    };

    enum class ClockSource : std::uint8_t { monotonic, absolute, boot, real };

    using Coroutine = std::coroutine_handle<BasePromise>;

    auto run() -> void;

    auto stop() noexcept -> void;

    auto spawn(Coroutine coroutine) -> void;

    template<std::movable T, typename F, typename... Args>
        requires std::is_invocable_r_v<Task<T>, F, Args...>
    constexpr auto spawn(F &&func, Args &&...args) {
        Task<T> task{std::invoke(func, std::forward<Args>(args)...)};

        std::future<T> result{std::move(task.getReturnValue())};
        const Coroutine coroutine{Coroutine::from_address(task.getCoroutine().address())};

        spawn(coroutine);

        return SpawnResult{std::hash<Coroutine>{}(coroutine), std::move(result)};
    }

    template<typename T, typename F, typename... Args>
        requires std::is_invocable_r_v<Task<T &>, F, Args...>
    constexpr auto spawn(F &&func, Args &&...args) {
        Task<T &> task{std::invoke(func, std::forward<Args>(args)...)};

        std::future<T &> result{std::move(task.getReturnValue())};
        const Coroutine coroutine{Coroutine::from_address(task.getCoroutine().address())};

        spawn(coroutine);

        return SpawnResult{std::hash<Coroutine>{}(coroutine), std::move(result)};
    }

    template<typename F, typename... Args>
        requires std::is_invocable_r_v<Task<>, F, Args...>
    constexpr auto spawn(F &&func, Args &&...args) {
        const Task<> task{std::invoke(func, std::forward<Args>(args)...)};

        const Coroutine coroutine{Coroutine::from_address(task.getCoroutine().address())};
        spawn(coroutine);

        return std::uint64_t{std::hash<Coroutine>{}(coroutine)};
    }

    [[nodiscard]] auto syncCancel(std::uint64_t taskIdentity, std::chrono::seconds seconds = {},
                                  std::chrono::nanoseconds nanoseconds = {}) -> std::int32_t;

    [[nodiscard]] auto syncCancel(std::int32_t fileDescriptor, bool isMatchAll = {}, std::chrono::seconds seconds = {},
                                  std::chrono::nanoseconds nanoseconds = {}) -> std::int32_t;

    [[nodiscard]] auto syncCancelAny(std::chrono::seconds seconds = {}, std::chrono::nanoseconds nanoseconds = {})
        -> std::int32_t;

    [[nodiscard]] auto cancel(std::uint64_t taskIdentity) -> AsyncWaiter;

    [[nodiscard]] auto cancel(std::int32_t fileDescriptor, bool isMatchAll = {}) -> AsyncWaiter;

    [[nodiscard]] auto cancelAny() -> AsyncWaiter;

    [[nodiscard]] auto sleep(std::chrono::seconds seconds = {}, std::chrono::nanoseconds nanoseconds = {},
                             ClockSource clockSource = {}) -> AsyncWaiter;

    [[nodiscard]] auto updateSleep(std::uint64_t taskIdentity, std::chrono::seconds seconds = {},
                                   std::chrono::nanoseconds nanoseconds = {}, ClockSource clockSource = {})
        -> AsyncWaiter;

    [[nodiscard]] auto timeout(AsyncWaiter &&asyncWaiter, std::chrono::seconds seconds = {},
                               std::chrono::nanoseconds nanoseconds = {}, ClockSource clockSource = {}) -> AsyncWaiter;

    [[nodiscard]] auto close(std::int32_t fileDescriptor) -> AsyncWaiter;

    [[nodiscard]] auto socket(std::int32_t domain, std::int32_t type, std::int32_t protocol) -> AsyncWaiter;

    [[nodiscard]] auto bind(std::int32_t socketFileDescriptor, sockaddr *address, socklen_t addressLength)
        -> AsyncWaiter;

    [[nodiscard]] auto listen(std::int32_t socketFileDescriptor, std::int32_t backlog) -> AsyncWaiter;

    [[nodiscard]] auto accept(std::int32_t socketFileDescriptor, sockaddr *address, socklen_t *addressLength,
                              std::int32_t flags = {}) -> AsyncWaiter;

    [[nodiscard]] auto connect(std::int32_t socketFileDescriptor, const sockaddr *address, socklen_t addressLength)
        -> AsyncWaiter;

    [[nodiscard]] auto shutdown(std::int32_t socketFileDescriptor, std::int32_t how) -> AsyncWaiter;

    [[nodiscard]] auto receive(std::int32_t socketFileDescriptor, std::span<std::byte> buffer, std::int32_t flags)
        -> AsyncWaiter;

    [[nodiscard]] auto receive(std::int32_t socketFileDescriptor, msghdr &message, std::uint32_t flags) -> AsyncWaiter;

    [[nodiscard]] auto send(std::int32_t socketFileDescriptor, std::span<const std::byte> buffer, std::int32_t flags)
        -> AsyncWaiter;

    [[nodiscard]] auto send(std::int32_t socketFileDescriptor, std::span<const std::byte> buffer, std::int32_t flags,
                            const sockaddr *address, socklen_t addressLength) -> AsyncWaiter;

    [[nodiscard]] auto send(std::int32_t socketFileDescriptor, const msghdr &message, std::uint32_t flags)
        -> AsyncWaiter;

    [[nodiscard]] auto open(std::string_view pathname, std::int32_t flags, mode_t mode = {}) -> AsyncWaiter;

    [[nodiscard]] auto open(std::int32_t directoryFileDescriptor, std::string_view pathname, std::int32_t flags,
                            mode_t mode = {}) -> AsyncWaiter;

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

    [[nodiscard]] auto link(std::string_view oldPath, std::string_view newPath) -> AsyncWaiter;

    [[nodiscard]] auto link(std::int32_t oldDirectoryFileDescriptor, std::string_view oldPath,
                            std::int32_t newDirectoryFileDescriptor, std::string_view newPath, std::int32_t flags)
        -> AsyncWaiter;

    [[nodiscard]] auto symbolicLink(std::string_view target, std::string_view linkPath) -> AsyncWaiter;

    [[nodiscard]] auto symbolicLink(std::string_view target, std::int32_t newDirectoryFileDescriptor,
                                    std::string_view linkPath) -> AsyncWaiter;
}    // namespace coContext

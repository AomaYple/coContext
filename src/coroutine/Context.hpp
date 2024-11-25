#pragma once

#include "../ring/Ring.hpp"

#include <string_view>
#include <variant>

namespace coContext {
    class Task;

    class Context {
    public:
        Context();

        Context(const Context &) = delete;

        auto operator=(const Context &) -> Context & = delete;

        Context(Context &&) noexcept = default;

        auto operator=(Context &&) noexcept -> Context & = default;

        ~Context() = default;

        auto swap(Context &other) noexcept -> void;

        auto spawn(Task &&task) -> void;

        auto run() -> void;

        auto stop() noexcept -> void;

        [[nodiscard]] auto cancel(std::variant<std::uint64_t, std::int32_t> identify, std::int32_t flags,
                                  __kernel_timespec timeout) -> std::int32_t;

        [[nodiscard]] auto cancel(std::uint64_t userData, std::int32_t flags) -> io_uring_sqe *;

        [[nodiscard]] auto cancel(std::int32_t fileDescriptor, std::int32_t flags) -> io_uring_sqe *;

        [[nodiscard]] auto close(std::int32_t fileDescriptor) -> io_uring_sqe *;

        [[nodiscard]] auto socket(std::int32_t domain, std::int32_t type, std::int32_t protocol,
                                  std::uint32_t flags = {}) -> io_uring_sqe *;

        [[nodiscard]] auto bind(std::int32_t socketFileDescriptor, sockaddr *address, std::uint32_t addressLength)
            -> io_uring_sqe *;

        [[nodiscard]] auto listen(std::int32_t socketFileDescriptor, std::int32_t backlog) -> io_uring_sqe *;

        [[nodiscard]] auto accept(std::int32_t socketFileDescriptor, sockaddr *address, std::uint32_t *addressLength,
                                  std::int32_t flags) -> io_uring_sqe *;

        [[nodiscard]] auto connect(std::int32_t socketFileDescriptor, const sockaddr *address,
                                   std::uint32_t addressLength) -> io_uring_sqe *;

        [[nodiscard]] auto shutdown(std::int32_t socketFileDescriptor, std::int32_t how) -> io_uring_sqe *;

        [[nodiscard]] auto receive(std::int32_t socketFileDescriptor, std::span<std::byte> buffer, std::int32_t flags)
            -> io_uring_sqe *;

        [[nodiscard]] auto receive(std::int32_t socketFileDescriptor, msghdr *message, std::uint32_t flags)
            -> io_uring_sqe *;

        [[nodiscard]] auto send(std::int32_t socketFileDescriptor, std::span<const std::byte> buffer,
                                std::int32_t flags) -> io_uring_sqe *;

        [[nodiscard]] auto send(std::int32_t socketFileDescriptor, std::span<const std::byte> buffer,
                                std::int32_t flags, const sockaddr *address, std::uint32_t addressLength)
            -> io_uring_sqe *;

        [[nodiscard]] auto send(std::int32_t socketFileDescriptor, const msghdr *message, std::uint32_t flags)
            -> io_uring_sqe *;

        [[nodiscard]] auto open(std::string_view pathname, std::int32_t flags, std::uint32_t mode) -> io_uring_sqe *;

        [[nodiscard]] auto open(std::int32_t directoryFileDescriptor, std::string_view pathname, std::int32_t flags,
                                std::uint32_t mode) -> io_uring_sqe *;

        [[nodiscard]] auto open(std::int32_t directoryFileDescriptor, std::string_view pathname, open_how *how)
            -> io_uring_sqe *;

        [[nodiscard]] auto read(std::int32_t fileDescriptor, std::span<std::byte> buffer, std::uint64_t offset)
            -> io_uring_sqe *;

        [[nodiscard]] auto read(std::int32_t fileDescriptor, std::span<const iovec> buffer, std::uint64_t offset)
            -> io_uring_sqe *;

        [[nodiscard]] auto read(std::int32_t fileDescriptor, std::span<const iovec> buffer, std::uint64_t offset,
                                std::int32_t flags) -> io_uring_sqe *;

        [[nodiscard]] auto write(std::int32_t fileDescriptor, std::span<const std::byte> buffer, std::uint64_t offset)
            -> io_uring_sqe *;

        [[nodiscard]] auto write(std::int32_t fileDescriptor, std::span<const iovec> buffer, std::uint64_t offset)
            -> io_uring_sqe *;

        [[nodiscard]] auto write(std::int32_t fileDescriptor, std::span<const iovec> buffer, std::uint64_t offset,
                                 std::int32_t flags) -> io_uring_sqe *;

    private:
        [[nodiscard]] static auto
            getFileDescriptorLimit(std::source_location sourceLocation = std::source_location::current())
                -> std::size_t;

        auto scheduleTasks() -> void;

        static constinit std::mutex mutex;
        static constinit std::int32_t sharedRingFileDescriptor;
        static constinit std::uint32_t cpuCode;

        Ring ring;
        bool isRunning{};
        std::vector<Task> unscheduledTasks;
        std::unordered_map<std::size_t, Task> schedulingTasks;
    };
}    // namespace coContext

template<>
constexpr auto std::swap(coContext::Context &lhs, coContext::Context &rhs) noexcept -> void {
    lhs.swap(rhs);
}

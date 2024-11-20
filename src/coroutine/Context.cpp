#include "coContext/coroutine/Context.hpp"

#include "../log/Exception.hpp"
#include "coContext/coroutine/AsyncWaiter.hpp"

#include <sys/resource.h>

coContext::Context::Context() :
    ring{[] {
        io_uring_params params{};
        params.flags = IORING_SETUP_SUBMIT_ALL | IORING_SETUP_COOP_TASKRUN | IORING_SETUP_TASKRUN_FLAG |
                       IORING_SETUP_SINGLE_ISSUER | IORING_SETUP_DEFER_TASKRUN;

        const std::lock_guard lock{mutex};

        if (sharedRingFileDescriptor != -1) {
            params.flags |= IORING_SETUP_ATTACH_WQ;
            params.wq_fd = sharedRingFileDescriptor;
        }

        Ring ring{static_cast<std::uint32_t>(getFileDescriptorLimit()) * 2, params};

        if (sharedRingFileDescriptor == -1) sharedRingFileDescriptor = ring.getFileDescriptor();

        return ring;
    }()} {
    this->ring.registerSelfFileDescriptor();

    constexpr cpu_set_t cpuSet{};
    {
        const std::lock_guard lock{mutex};

        CPU_SET(cpuCode++, &cpuSet);
        cpuCode %= std::jthread::hardware_concurrency();
    }
    this->ring.registerCpuAffinity(sizeof(cpuSet), &cpuSet);
}

auto coContext::Context::run() -> void {
    while (true) {
        this->ring.submitAndWait(1);
        this->ring.advance(this->ring.poll([this](const io_uring_cqe *const completionQueueEntry) {
            const Task &task{this->tasks.at(completionQueueEntry->user_data)};
            task(completionQueueEntry->res);

            if (task.done()) this->tasks.erase(completionQueueEntry->user_data);
        }));
    }
}

auto coContext::Context::submit(Task &&task) -> void { this->tasks.emplace(task.getHash(), std::move(task)); }

auto coContext::Context::close(const std::int32_t fileDescriptor) -> AsyncWaiter {
    io_uring_sqe *const submissionQueueEntry{this->ring.getSubmissionQueueEntry()};
    io_uring_prep_close(submissionQueueEntry, fileDescriptor);

    return AsyncWaiter{submissionQueueEntry};
}

auto coContext::Context::socket(const std::int32_t domain, const std::int32_t type, const std::int32_t protocol)
    -> AsyncWaiter {
    io_uring_sqe *const submissionQueueEntry{this->ring.getSubmissionQueueEntry()};
    io_uring_prep_socket(submissionQueueEntry, domain, type, protocol, 0);

    return AsyncWaiter{submissionQueueEntry};
}

auto coContext::Context::bind(const std::int32_t socketFileDescriptor, sockaddr *const address,
                              const std::uint32_t addressLength) -> AsyncWaiter {
    io_uring_sqe *const submissionQueueEntry{this->ring.getSubmissionQueueEntry()};
    io_uring_prep_bind(submissionQueueEntry, socketFileDescriptor, address, addressLength);

    return AsyncWaiter{submissionQueueEntry};
}

auto coContext::Context::listen(const std::int32_t socketFileDescriptor, const std::int32_t backlog) -> AsyncWaiter {
    io_uring_sqe *const submissionQueueEntry{this->ring.getSubmissionQueueEntry()};
    io_uring_prep_listen(submissionQueueEntry, socketFileDescriptor, backlog);

    return AsyncWaiter{submissionQueueEntry};
}

auto coContext::Context::accept(const std::int32_t socketFileDescriptor, sockaddr *const address,
                                std::uint32_t *const addressLength, const std::int32_t flags) -> AsyncWaiter {
    io_uring_sqe *const submissionQueueEntry{this->ring.getSubmissionQueueEntry()};
    io_uring_prep_accept(submissionQueueEntry, socketFileDescriptor, address, addressLength, flags);
    submissionQueueEntry->ioprio |= IORING_ACCEPT_POLL_FIRST;

    return AsyncWaiter{submissionQueueEntry};
}

auto coContext::Context::connect(const std::int32_t socketFileDescriptor, const sockaddr *const address,
                                 const std::uint32_t addressLength) -> AsyncWaiter {
    io_uring_sqe *const submissionQueueEntry{this->ring.getSubmissionQueueEntry()};
    io_uring_prep_connect(submissionQueueEntry, socketFileDescriptor, address, addressLength);

    return AsyncWaiter{submissionQueueEntry};
}

auto coContext::Context::shutdown(const std::int32_t socketFileDescriptor, const std::int32_t how) -> AsyncWaiter {
    io_uring_sqe *const submissionQueueEntry{this->ring.getSubmissionQueueEntry()};
    io_uring_prep_shutdown(submissionQueueEntry, socketFileDescriptor, how);

    return AsyncWaiter{submissionQueueEntry};
}

auto coContext::Context::receive(const std::int32_t socketFileDescriptor, const std::span<std::byte> buffer,
                                 const std::int32_t flags) -> AsyncWaiter {
    io_uring_sqe *const submissionQueueEntry{this->ring.getSubmissionQueueEntry()};
    io_uring_prep_recv(submissionQueueEntry, socketFileDescriptor, buffer.data(), buffer.size(), flags);
    submissionQueueEntry->ioprio |= IORING_RECVSEND_POLL_FIRST;

    return AsyncWaiter{submissionQueueEntry};
}

auto coContext::Context::receive(const std::int32_t socketFileDescriptor, msghdr *const message,
                                 const std::uint32_t flags) -> AsyncWaiter {
    io_uring_sqe *const submissionQueueEntry{this->ring.getSubmissionQueueEntry()};
    io_uring_prep_recvmsg(submissionQueueEntry, socketFileDescriptor, message, flags);
    submissionQueueEntry->ioprio |= IORING_RECVSEND_POLL_FIRST;

    return AsyncWaiter{submissionQueueEntry};
}

auto coContext::Context::send(const std::int32_t socketFileDescriptor, const std::span<const std::byte> buffer,
                              const std::int32_t flags) -> AsyncWaiter {
    io_uring_sqe *const submissionQueueEntry{this->ring.getSubmissionQueueEntry()};
    io_uring_prep_send(submissionQueueEntry, socketFileDescriptor, buffer.data(), buffer.size(), flags);
    submissionQueueEntry->ioprio |= IORING_RECVSEND_POLL_FIRST;

    return AsyncWaiter{submissionQueueEntry};
}

auto coContext::Context::send(const std::int32_t socketFileDescriptor, const std::span<const std::byte> buffer,
                              const std::int32_t flags, const sockaddr *const address,
                              const std::uint32_t addressLength) -> AsyncWaiter {
    io_uring_sqe *const submissionQueueEntry{this->ring.getSubmissionQueueEntry()};
    io_uring_prep_sendto(submissionQueueEntry, socketFileDescriptor, buffer.data(), buffer.size(), flags, address,
                         addressLength);

    return AsyncWaiter{submissionQueueEntry};
}

auto coContext::Context::send(const std::int32_t socketFileDescriptor, const msghdr *const message,
                              const std::uint32_t flags) -> AsyncWaiter {
    io_uring_sqe *const submissionQueueEntry{this->ring.getSubmissionQueueEntry()};
    io_uring_prep_sendmsg(submissionQueueEntry, socketFileDescriptor, message, flags);
    submissionQueueEntry->ioprio |= IORING_RECVSEND_POLL_FIRST;

    return AsyncWaiter{submissionQueueEntry};
}

auto coContext::Context::open(const std::string_view pathname, const std::int32_t flags, const std::uint32_t mode)
    -> AsyncWaiter {
    io_uring_sqe *const submissionQueueEntry{this->ring.getSubmissionQueueEntry()};
    io_uring_prep_open(submissionQueueEntry, pathname.data(), flags, mode);

    return AsyncWaiter{submissionQueueEntry};
}

auto coContext::Context::open(const std::int32_t directoryFileDescriptor, const std::string_view pathname,
                              const std::int32_t flags, const std::uint32_t mode) -> AsyncWaiter {
    io_uring_sqe *const submissionQueueEntry{this->ring.getSubmissionQueueEntry()};
    io_uring_prep_openat(submissionQueueEntry, directoryFileDescriptor, pathname.data(), flags, mode);

    return AsyncWaiter{submissionQueueEntry};
}

auto coContext::Context::getFileDescriptorLimit(const std::source_location sourceLocation) -> std::uint64_t {
    rlimit limit{};
    if (getrlimit(RLIMIT_NOFILE, &limit) == -1) {
        throw Exception{
            Log{Log::Level::fatal, std::error_code{errno, std::generic_category()}.message(), sourceLocation}
        };
    }

    return limit.rlim_cur;
}

std::mutex coContext::Context::mutex;
std::int32_t coContext::Context::sharedRingFileDescriptor{-1};
std::uint32_t coContext::Context::cpuCode;

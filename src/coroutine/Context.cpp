#include "Context.hpp"

#include "../log/Exception.hpp"
#include "../log/Log.hpp"
#include "coContext/coroutine/Task.hpp"

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
    constexpr cpu_set_t cpuSet{};
    {
        const std::lock_guard lock{mutex};

        CPU_SET(cpuCode++, &cpuSet);
        cpuCode %= std::thread::hardware_concurrency();
    }
    this->ring.registerCpuAffinity(cpuSet);

    this->ring.registerSelfFileDescriptor();
}

auto coContext::Context::swap(Context &other) noexcept -> void {
    std::swap(this->ring, other.ring);
    std::swap(this->schedulingTasks, other.schedulingTasks);
}

auto coContext::Context::spawn(Task &&task) -> void { this->unscheduledTasks.emplace_back(std::move(task)); }

auto coContext::Context::run() -> void {
    this->isRunning = true;

    while (this->isRunning) {
        this->scheduleTasks();

        this->ring.submitAndWait(1);
        this->ring.advance(this->ring.poll([this](const io_uring_cqe *const completionQueueEntry) {
            const Task &task{this->schedulingTasks.at(completionQueueEntry->user_data)};
            task(completionQueueEntry->res);

            if (task.done()) this->schedulingTasks.erase(completionQueueEntry->user_data);
        }));
    }
}

auto coContext::Context::stop() -> io_uring_sqe * {
    this->isRunning = false;

    return this->cancel(std::uint64_t{}, IORING_ASYNC_CANCEL_ALL);
}

auto coContext::Context::cancel(const std::variant<std::uint64_t, std::int32_t> identify, const std::int32_t flags,
                                const __kernel_timespec timeout) -> std::int32_t {
    io_uring_sync_cancel_reg parameters{};

    if (std::holds_alternative<std::uint64_t>(identify)) parameters.addr = std::get<std::uint64_t>(identify);
    else {
        parameters.fd = std::get<std::int32_t>(identify);
        parameters.flags = IORING_ASYNC_CANCEL_FD;
    }

    parameters.flags |= flags;
    parameters.timeout = timeout;

    return this->ring.syncCancel(parameters);
}

auto coContext::Context::cancel(const std::uint64_t userData, const std::int32_t flags) -> io_uring_sqe * {
    io_uring_sqe *const submissionQueueEntry{this->ring.getSubmissionQueueEntry()};
    io_uring_prep_cancel64(submissionQueueEntry, userData, flags);

    return submissionQueueEntry;
}

auto coContext::Context::cancel(const std::int32_t fileDescriptor, const std::int32_t flags) -> io_uring_sqe * {
    io_uring_sqe *const submissionQueueEntry{this->ring.getSubmissionQueueEntry()};
    io_uring_prep_cancel_fd(submissionQueueEntry, fileDescriptor, flags);

    return submissionQueueEntry;
}

auto coContext::Context::close(const std::int32_t fileDescriptor) -> io_uring_sqe * {
    io_uring_sqe *const submissionQueueEntry{this->ring.getSubmissionQueueEntry()};
    io_uring_prep_close(submissionQueueEntry, fileDescriptor);

    return submissionQueueEntry;
}

auto coContext::Context::socket(const std::int32_t domain, const std::int32_t type, const std::int32_t protocol,
                                const std::uint32_t flags) -> io_uring_sqe * {
    io_uring_sqe *const submissionQueueEntry{this->ring.getSubmissionQueueEntry()};
    io_uring_prep_socket(submissionQueueEntry, domain, type, protocol, flags);

    return submissionQueueEntry;
}

auto coContext::Context::bind(const std::int32_t socketFileDescriptor, sockaddr *const address,
                              const std::uint32_t addressLength) -> io_uring_sqe * {
    io_uring_sqe *const submissionQueueEntry{this->ring.getSubmissionQueueEntry()};
    io_uring_prep_bind(submissionQueueEntry, socketFileDescriptor, address, addressLength);

    return submissionQueueEntry;
}

auto coContext::Context::listen(const std::int32_t socketFileDescriptor, const std::int32_t backlog) -> io_uring_sqe * {
    io_uring_sqe *const submissionQueueEntry{this->ring.getSubmissionQueueEntry()};
    io_uring_prep_listen(submissionQueueEntry, socketFileDescriptor, backlog);

    return submissionQueueEntry;
}

auto coContext::Context::accept(const std::int32_t socketFileDescriptor, sockaddr *const address,
                                std::uint32_t *const addressLength, const std::int32_t flags) -> io_uring_sqe * {
    io_uring_sqe *const submissionQueueEntry{this->ring.getSubmissionQueueEntry()};
    io_uring_prep_accept(submissionQueueEntry, socketFileDescriptor, address, addressLength, flags);
    submissionQueueEntry->ioprio |= IORING_ACCEPT_POLL_FIRST;

    return submissionQueueEntry;
}

auto coContext::Context::connect(const std::int32_t socketFileDescriptor, const sockaddr *const address,
                                 const std::uint32_t addressLength) -> io_uring_sqe * {
    io_uring_sqe *const submissionQueueEntry{this->ring.getSubmissionQueueEntry()};
    io_uring_prep_connect(submissionQueueEntry, socketFileDescriptor, address, addressLength);

    return submissionQueueEntry;
}

auto coContext::Context::shutdown(const std::int32_t socketFileDescriptor, const std::int32_t how) -> io_uring_sqe * {
    io_uring_sqe *const submissionQueueEntry{this->ring.getSubmissionQueueEntry()};
    io_uring_prep_shutdown(submissionQueueEntry, socketFileDescriptor, how);

    return submissionQueueEntry;
}

auto coContext::Context::receive(const std::int32_t socketFileDescriptor, const std::span<std::byte> buffer,
                                 const std::int32_t flags) -> io_uring_sqe * {
    io_uring_sqe *const submissionQueueEntry{this->ring.getSubmissionQueueEntry()};
    io_uring_prep_recv(submissionQueueEntry, socketFileDescriptor, std::data(buffer), std::size(buffer), flags);
    submissionQueueEntry->ioprio |= IORING_RECVSEND_POLL_FIRST;

    return submissionQueueEntry;
}

auto coContext::Context::receive(const std::int32_t socketFileDescriptor, msghdr *const message,
                                 const std::uint32_t flags) -> io_uring_sqe * {
    io_uring_sqe *const submissionQueueEntry{this->ring.getSubmissionQueueEntry()};
    io_uring_prep_recvmsg(submissionQueueEntry, socketFileDescriptor, message, flags);
    submissionQueueEntry->ioprio |= IORING_RECVSEND_POLL_FIRST;

    return submissionQueueEntry;
}

auto coContext::Context::send(const std::int32_t socketFileDescriptor, const std::span<const std::byte> buffer,
                              const std::int32_t flags) -> io_uring_sqe * {
    io_uring_sqe *const submissionQueueEntry{this->ring.getSubmissionQueueEntry()};
    io_uring_prep_send(submissionQueueEntry, socketFileDescriptor, std::data(buffer), std::size(buffer), flags);
    submissionQueueEntry->ioprio |= IORING_RECVSEND_POLL_FIRST;

    return submissionQueueEntry;
}

auto coContext::Context::send(const std::int32_t socketFileDescriptor, const std::span<const std::byte> buffer,
                              const std::int32_t flags, const sockaddr *const address,
                              const std::uint32_t addressLength) -> io_uring_sqe * {
    io_uring_sqe *const submissionQueueEntry{this->ring.getSubmissionQueueEntry()};
    io_uring_prep_sendto(submissionQueueEntry, socketFileDescriptor, std::data(buffer), std::size(buffer), flags,
                         address, addressLength);

    return submissionQueueEntry;
}

auto coContext::Context::send(const std::int32_t socketFileDescriptor, const msghdr *const message,
                              const std::uint32_t flags) -> io_uring_sqe * {
    io_uring_sqe *const submissionQueueEntry{this->ring.getSubmissionQueueEntry()};
    io_uring_prep_sendmsg(submissionQueueEntry, socketFileDescriptor, message, flags);
    submissionQueueEntry->ioprio |= IORING_RECVSEND_POLL_FIRST;

    return submissionQueueEntry;
}

auto coContext::Context::open(const std::string_view pathname, const std::int32_t flags, const std::uint32_t mode)
    -> io_uring_sqe * {
    io_uring_sqe *const submissionQueueEntry{this->ring.getSubmissionQueueEntry()};
    io_uring_prep_open(submissionQueueEntry, std::data(pathname), flags, mode);

    return submissionQueueEntry;
}

auto coContext::Context::open(const std::int32_t directoryFileDescriptor, const std::string_view pathname,
                              const std::int32_t flags, const std::uint32_t mode) -> io_uring_sqe * {
    io_uring_sqe *const submissionQueueEntry{this->ring.getSubmissionQueueEntry()};
    io_uring_prep_openat(submissionQueueEntry, directoryFileDescriptor, std::data(pathname), flags, mode);

    return submissionQueueEntry;
}

auto coContext::Context::open(const std::int32_t directoryFileDescriptor, const std::string_view pathname,
                              open_how *const how) -> io_uring_sqe * {
    io_uring_sqe *const submissionQueueEntry{this->ring.getSubmissionQueueEntry()};
    io_uring_prep_openat2(submissionQueueEntry, directoryFileDescriptor, std::data(pathname), how);

    return submissionQueueEntry;
}

auto coContext::Context::read(const std::int32_t fileDescriptor, const std::span<std::byte> buffer,
                              const std::uint64_t offset) -> io_uring_sqe * {
    io_uring_sqe *const submissionQueueEntry{this->ring.getSubmissionQueueEntry()};
    io_uring_prep_read(submissionQueueEntry, fileDescriptor, std::data(buffer), std::size(buffer), offset);

    return submissionQueueEntry;
}

auto coContext::Context::read(const std::int32_t fileDescriptor, const std::span<const iovec> buffer,
                              const std::uint64_t offset) -> io_uring_sqe * {
    io_uring_sqe *const submissionQueueEntry{this->ring.getSubmissionQueueEntry()};
    io_uring_prep_readv(submissionQueueEntry, fileDescriptor, std::data(buffer), std::size(buffer), offset);

    return submissionQueueEntry;
}

auto coContext::Context::read(const std::int32_t fileDescriptor, const std::span<const iovec> buffer,
                              const std::uint64_t offset, const std::int32_t flags) -> io_uring_sqe * {
    io_uring_sqe *const submissionQueueEntry{this->ring.getSubmissionQueueEntry()};
    io_uring_prep_readv2(submissionQueueEntry, fileDescriptor, std::data(buffer), std::size(buffer), offset, flags);

    return submissionQueueEntry;
}

auto coContext::Context::write(const std::int32_t fileDescriptor, const std::span<const std::byte> buffer,
                               const std::uint64_t offset) -> io_uring_sqe * {
    io_uring_sqe *const submissionQueueEntry{this->ring.getSubmissionQueueEntry()};
    io_uring_prep_write(submissionQueueEntry, fileDescriptor, std::data(buffer), std::size(buffer), offset);

    return submissionQueueEntry;
}

auto coContext::Context::write(const std::int32_t fileDescriptor, const std::span<const iovec> buffer,
                               const std::uint64_t offset) -> io_uring_sqe * {
    io_uring_sqe *const submissionQueueEntry{this->ring.getSubmissionQueueEntry()};
    io_uring_prep_writev(submissionQueueEntry, fileDescriptor, std::data(buffer), std::size(buffer), offset);

    return submissionQueueEntry;
}

auto coContext::Context::write(const std::int32_t fileDescriptor, const std::span<const iovec> buffer,
                               const std::uint64_t offset, const std::int32_t flags) -> io_uring_sqe * {
    io_uring_sqe *const submissionQueueEntry{this->ring.getSubmissionQueueEntry()};
    io_uring_prep_writev2(submissionQueueEntry, fileDescriptor, std::data(buffer), std::size(buffer), offset, flags);

    return submissionQueueEntry;
}

auto coContext::Context::getFileDescriptorLimit(const std::source_location sourceLocation) -> std::size_t {
    rlimit limit{};
    if (getrlimit(RLIMIT_NOFILE, std::addressof(limit)) == -1) {
        throw Exception{
            Log{Log::Level::fatal, std::error_code{errno, std::generic_category()}.message(), sourceLocation}
                .toString()
        };
    }

    return limit.rlim_cur;
}

auto coContext::Context::scheduleTasks() -> void {
    for (auto &task : this->unscheduledTasks) {
        task(0);
        this->schedulingTasks.emplace(task.getHash(), std::move(task));
    }

    this->unscheduledTasks.clear();
}

std::mutex coContext::Context::mutex;
std::int32_t coContext::Context::sharedRingFileDescriptor{-1};
std::uint32_t coContext::Context::cpuCode;

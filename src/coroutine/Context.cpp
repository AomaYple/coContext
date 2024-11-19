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

auto coContext::Context::submit(Task &&task) -> void {
    const std::uint64_t hashValue{task.getHash()};

    io_uring_sqe_set_data64(task.getSubmissionQueueEntry(), hashValue);
    this->tasks.emplace(hashValue, std::move(task));
}

auto coContext::Context::close(const int fileDescriptor) -> AsyncWaiter {
    io_uring_sqe *const submissionQueueEntry{this->ring.getSubmissionQueueEntry()};
    io_uring_prep_close(submissionQueueEntry, fileDescriptor);

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

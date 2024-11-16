#include "coContext/coroutine/Context.hpp"

#include "../log/Exception.hpp"

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

        Ring ring{static_cast<unsigned int>(getFileDescriptorLimit()) * 2, params};

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

auto coContext::Context::getFileDescriptorLimit(const std::source_location sourceLocation) -> unsigned long {
    rlimit limit{};
    if (getrlimit(RLIMIT_NOFILE, &limit) == -1) {
        throw Exception{
            Log{Log::Level::fatal, std::error_code{errno, std::generic_category()}.message(), sourceLocation}
        };
    }

    return limit.rlim_cur;
}

std::mutex coContext::Context::mutex;
int coContext::Context::sharedRingFileDescriptor{-1};
unsigned int coContext::Context::cpuCode;

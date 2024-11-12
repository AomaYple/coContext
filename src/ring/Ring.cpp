#include "ring/Ring.hpp"

#include "../log/Exception.hpp"

coContext::Ring::Ring(const unsigned int entries, io_uring_params &params) :
    handle{[entries, &params](const std::source_location sourceLocation = std::source_location::current()) {
        io_uring handle{};
        if (const int result{io_uring_queue_init_params(entries, &handle, &params)}; result != 0) {
            throw Exception{
                Log{Log::Level::fatal, std::error_code{std::abs(result), std::generic_category()}.message(),
                    sourceLocation}
            };
        }

        return handle;
    }()} {}

coContext::Ring::Ring(Ring &&other) noexcept : handle{other.handle} { other.handle.ring_fd = -1; }

auto coContext::Ring::operator=(Ring &&other) noexcept -> Ring & {
    if (this == &other) return *this;

    this->destroy();

    this->handle = other.handle;
    other.handle.ring_fd = -1;

    return *this;
}

coContext::Ring::~Ring() { this->destroy(); }

auto coContext::Ring::getFileDescriptor() const noexcept -> int { return this->handle.ring_fd; }

auto coContext::Ring::registerSelfFileDescriptor(const std::source_location sourceLocation) -> void {
    if (const int result{io_uring_register_ring_fd(&this->handle)}; result != 1) {
        throw Exception{
            Log{Log::Level::error, std::error_code{std::abs(result), std::generic_category()}.message(),
                sourceLocation}
        };
    }
}

auto coContext::Ring::registerCpuAffinity(const cpu_set_t &cpuSet, const std::source_location sourceLocation) -> void {
    if (const int result{io_uring_register_iowq_aff(&this->handle, sizeof(cpuSet), &cpuSet)}; result != 0) {
        throw Exception{
            Log{Log::Level::error, std::error_code{std::abs(result), std::generic_category()}.message(),
                sourceLocation}
        };
    }
}

auto coContext::Ring::getSqe(const std::source_location sourceLocation) -> io_uring_sqe * {
    io_uring_sqe *const sqe{io_uring_get_sqe(&this->handle)};
    if (sqe == nullptr) {
        throw Exception{
            Log{Log::Level::error, "no sqe available", sourceLocation}
        };
    }

    return sqe;
}

auto coContext::Ring::submitAndWait(const unsigned int count, const std::source_location sourceLocation) -> void {
    if (const int result{io_uring_submit_and_wait(&this->handle, count)}; result < 0) {
        throw Exception{
            Log{Log::Level::error, std::error_code{std::abs(result), std::generic_category()}.message(),
                sourceLocation}
        };
    }
}

auto coContext::Ring::poll(std::move_only_function<auto(const io_uring_cqe *)->void> &&action) const -> int {
    int count{};

    unsigned int head;
    const io_uring_cqe *cqe;
    io_uring_for_each_cqe(&this->handle, head, cqe) {
        action(cqe);
        ++count;
    }

    return count;
}

auto coContext::Ring::advance(io_uring_buf_ring *ringBuffer, const int cqeCount, const int bufferCount) noexcept
    -> void {
    __io_uring_buf_ring_cq_advance(&this->handle, ringBuffer, cqeCount, bufferCount);
}

auto coContext::Ring::destroy() noexcept -> void {
    if (this->handle.ring_fd != -1) io_uring_queue_exit(&this->handle);
}

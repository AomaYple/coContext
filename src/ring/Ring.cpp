#include "Ring.hpp"

#include "../log/Exception.hpp"

using namespace std::string_view_literals;

coContext::Ring::Ring(const std::uint32_t entries, io_uring_params &parameters) :
    handle{[entries, &parameters](const std::source_location sourceLocation = std::source_location::current()) {
        io_uring handle{};
        if (const std::int32_t result{
                io_uring_queue_init_params(entries, std::addressof(handle), std::addressof(parameters))};
            result != 0) {
            throw Exception{
                Log{Log::Level::fatal,
                    std::pmr::string{std::error_code{std::abs(result), std::generic_category()}.message(),
                                     getMemoryResource()},
                    sourceLocation}
            };
        }

        return handle;
    }()} {}

coContext::Ring::Ring(Ring &&other) noexcept : handle{other.handle} { other.handle.ring_fd = -1; }

auto coContext::Ring::operator=(Ring &&other) noexcept -> Ring & {
    if (this == std::addressof(other)) return *this;

    this->destroy();

    this->handle = other.handle;
    other.handle.ring_fd = -1;

    return *this;
}

coContext::Ring::~Ring() { this->destroy(); }

auto coContext::Ring::swap(Ring &other) noexcept -> void { std::swap(this->handle, other.handle); }

auto coContext::Ring::getFileDescriptor() const noexcept -> std::int32_t { return this->handle.ring_fd; }

auto coContext::Ring::registerSelfFileDescriptor(const std::source_location sourceLocation) -> void {
    if (const std::int32_t result{io_uring_register_ring_fd(std::addressof(this->handle))}; result != 1) {
        throw Exception{
            Log{Log::Level::error,
                std::pmr::string{std::error_code{std::abs(result), std::generic_category()}.message(),
                                 getMemoryResource()},
                sourceLocation}
        };
    }
}

auto coContext::Ring::registerSparseFileDescriptor(const std::uint32_t count, const std::source_location sourceLocation)
    -> void {
    if (const std::int32_t result{io_uring_register_files_sparse(std::addressof(this->handle), count)}; result != 0) {
        throw Exception{
            Log{Log::Level::error,
                std::pmr::string{std::error_code{std::abs(result), std::generic_category()}.message(),
                                 getMemoryResource()},
                sourceLocation}
        };
    }
}

auto coContext::Ring::registerCpuAffinity(const std::size_t cpuSetSize, const cpu_set_t *const cpuSet,
                                          const std::source_location sourceLocation) -> void {
    if (const std::int32_t result{io_uring_register_iowq_aff(std::addressof(this->handle), cpuSetSize, cpuSet)};
        result != 0) {
        throw Exception{
            Log{Log::Level::error,
                std::pmr::string{std::error_code{std::abs(result), std::generic_category()}.message(),
                                 getMemoryResource()},
                sourceLocation}
        };
    }
}

auto coContext::Ring::setupRingBuffer(const std::uint32_t entries, const std::int32_t id, const std::uint32_t flags,
                                      const std::source_location sourceLocation) -> io_uring_buf_ring * {
    std::int32_t error;
    io_uring_buf_ring *const handle{
        io_uring_setup_buf_ring(std::addressof(this->handle), entries, id, flags, std::addressof(error))};
    if (handle == nullptr) {
        throw Exception{
            Log{Log::Level::error,
                std::pmr::string{std::error_code{std::abs(error), std::generic_category()}.message(),
                                 getMemoryResource()},
                sourceLocation}
        };
    }

    return handle;
}

auto coContext::Ring::freeRingBuffer(io_uring_buf_ring *const ringBuffer, const std::uint32_t entries,
                                     const std::int32_t id, const std::source_location sourceLocation) -> void {
    if (const std::int32_t result{io_uring_free_buf_ring(std::addressof(this->handle), ringBuffer, entries, id)};
        result != 0) {
        throw Exception{
            Log{Log::Level::error,
                std::pmr::string{std::error_code{std::abs(result), std::generic_category()}.message(),
                                 getMemoryResource()},
                sourceLocation}
        };
    }
}

auto coContext::Ring::getSubmissionQueueEntry(const std::source_location sourceLocation) -> io_uring_sqe * {
    io_uring_sqe *const submissionQueueEntry{io_uring_get_sqe(std::addressof(this->handle))};
    if (submissionQueueEntry == nullptr) {
        throw Exception{
            Log{Log::Level::error, std::pmr::string{"no submission queue entry available"sv, getMemoryResource()},
                sourceLocation}
        };
    }

    return submissionQueueEntry;
}

auto coContext::Ring::submitAndWait(const std::uint32_t count, const std::source_location sourceLocation) -> void {
    if (const std::int32_t result{io_uring_submit_and_wait(std::addressof(this->handle), count)}; result < 0) {
        throw Exception{
            Log{Log::Level::error,
                std::pmr::string{std::error_code{std::abs(result), std::generic_category()}.message(),
                                 getMemoryResource()},
                sourceLocation}
        };
    }
}

auto coContext::Ring::poll(std::move_only_function<auto(const io_uring_cqe *)->void> &&action) const -> std::int32_t {
    std::int32_t count{};

    std::uint32_t head;
    const io_uring_cqe *completionQueueEntry;
    io_uring_for_each_cqe(std::addressof(this->handle), head, completionQueueEntry) {
        action(completionQueueEntry);
        ++count;
    }

    return count;
}

auto coContext::Ring::advance(const std::uint32_t count) noexcept -> void {
    io_uring_cq_advance(std::addressof(this->handle), count);
}

auto coContext::Ring::advance(io_uring_buf_ring *const ringBuffer, const std::int32_t completionQueueEntryCount,
                              const std::int32_t ringBufferBufferCount) noexcept -> void {
    __io_uring_buf_ring_cq_advance(std::addressof(this->handle), ringBuffer, completionQueueEntryCount,
                                   ringBufferBufferCount);
}

auto coContext::Ring::syncCancel(io_uring_sync_cancel_reg &parameters, const std::source_location sourceLocation)
    -> std::int32_t {
    const std::int32_t result{io_uring_register_sync_cancel(std::addressof(this->handle), std::addressof(parameters))};
    if (result < 0) {
        throw Exception{
            Log{Log::Level::warn,
                std::pmr::string{std::error_code{std::abs(result), std::generic_category()}.message(),
                                 getMemoryResource()},
                sourceLocation}
        };
    }

    return result;
}

auto coContext::Ring::destroy() noexcept -> void {
    if (this->handle.ring_fd != -1) io_uring_queue_exit(std::addressof(this->handle));
}

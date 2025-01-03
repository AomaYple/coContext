#include "Ring.hpp"

#include "../log/Exception.hpp"

using namespace std::string_view_literals;

coContext::internal::Ring::Ring(const std::uint32_t entries, io_uring_params &parameters) :
    handle{[entries, &parameters](const std::source_location sourceLocation = std::source_location::current()) {
        io_uring handle;
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

coContext::internal::Ring::Ring(Ring &&other) noexcept : handle{other.handle} { other.handle.ring_fd = -1; }

auto coContext::internal::Ring::operator=(Ring &&other) noexcept -> Ring & {
    if (this == std::addressof(other)) return *this;

    this->~Ring();

    this->handle = other.handle;
    other.handle.ring_fd = -1;

    return *this;
}

coContext::internal::Ring::~Ring() {
    if (this->handle.ring_fd != -1) io_uring_queue_exit(std::addressof(this->handle));
}

auto coContext::internal::Ring::swap(Ring &other) noexcept -> void { std::swap(this->handle, other.handle); }

auto coContext::internal::Ring::getFileDescriptor() const noexcept -> std::int32_t { return this->handle.ring_fd; }

auto coContext::internal::Ring::registerSelfFileDescriptor(const std::source_location sourceLocation) -> void {
    if (const std::int32_t result{io_uring_register_ring_fd(std::addressof(this->handle))}; result != 1) {
        throw Exception{
            Log{Log::Level::error,
                std::pmr::string{std::error_code{std::abs(result), std::generic_category()}.message(),
                                 getMemoryResource()},
                sourceLocation}
        };
    }
}

auto coContext::internal::Ring::registerSparseFileDescriptor(const std::uint32_t count,
                                                             const std::source_location sourceLocation) -> void {
    if (const std::int32_t result{io_uring_register_files_sparse(std::addressof(this->handle), count)}; result != 0) {
        throw Exception{
            Log{Log::Level::error,
                std::pmr::string{std::error_code{std::abs(result), std::generic_category()}.message(),
                                 getMemoryResource()},
                sourceLocation}
        };
    }
}

auto coContext::internal::Ring::setupRingBuffer(const std::uint32_t entries, const std::int32_t id,
                                                const std::uint32_t flags, const std::source_location sourceLocation)
    -> io_uring_buf_ring * {
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

auto coContext::internal::Ring::freeRingBuffer(io_uring_buf_ring *const ringBuffer, const std::uint32_t entries,
                                               const std::int32_t id, const std::source_location sourceLocation)
    -> void {
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

auto coContext::internal::Ring::getSubmission(const std::source_location sourceLocation) -> io_uring_sqe * {
    io_uring_sqe *const submission{io_uring_get_sqe(std::addressof(this->handle))};
    if (submission == nullptr) {
        throw Exception{
            Log{Log::Level::error, std::pmr::string{"no submission available"sv, getMemoryResource()}, sourceLocation}
        };
    }

    return submission;
}

auto coContext::internal::Ring::submitAndWait(const std::uint32_t count, const std::source_location sourceLocation)
    -> void {
    if (const std::int32_t result{io_uring_submit_and_wait(std::addressof(this->handle), count)}; result < 0) {
        throw Exception{
            Log{Log::Level::error,
                std::pmr::string{std::error_code{std::abs(result), std::generic_category()}.message(),
                                 getMemoryResource()},
                sourceLocation}
        };
    }
}

auto coContext::internal::Ring::poll(std::move_only_function<auto(const io_uring_cqe *)->void> action) const
    -> std::int32_t {
    std::int32_t count{};

    std::uint32_t head;
    const io_uring_cqe *completion;
    io_uring_for_each_cqe(std::addressof(this->handle), head, completion) {
        action(completion);
        ++count;
    }

    return count;
}

auto coContext::internal::Ring::advance(io_uring_buf_ring *const ringBuffer, const std::int32_t completionCount,
                                        const std::int32_t bufferCount) noexcept -> void {
    __io_uring_buf_ring_cq_advance(std::addressof(this->handle), ringBuffer, completionCount, bufferCount);
}

auto coContext::internal::Ring::syncCancel(io_uring_sync_cancel_reg &parameters,
                                           const std::source_location sourceLocation) -> std::int32_t {
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

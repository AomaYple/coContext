#include "ring/Ring.hpp"

#include "../exception/Exception.hpp"

#include <system_error>

coContext::Ring::Ring(const unsigned int entries, io_uring_params &params) :
    handle{[entries, &params] {
        io_uring handle{};
        if (const int result{io_uring_queue_init_params(entries, &handle, &params)}; result != 0) {
            throw Exception{
                std::error_code{std::abs(result), std::generic_category()}
                .message()
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

auto coContext::Ring::destroy() noexcept -> void {
    if (this->handle.ring_fd != -1) io_uring_queue_exit(&this->handle);
}

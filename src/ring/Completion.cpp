#include "Completion.hpp"

#include <liburing.h>

coContext::internal::Completion::Completion(const io_uring_cqe *const handle) noexcept : handle{handle} {}

auto coContext::internal::Completion::get() const noexcept -> const io_uring_cqe * { return this->handle; }

auto coContext::internal::Completion::getUserData() const noexcept -> std::uint64_t {
    return io_uring_cqe_get_data64(this->handle);
}

auto coContext::internal::Completion::getResult() const noexcept -> std::int32_t { return this->handle->res; }

auto coContext::internal::Completion::getFlags() const noexcept -> std::uint32_t { return this->handle->flags; }

auto coContext::internal::operator==(const Completion lhs, const Completion rhs) noexcept -> bool {
    return lhs.get() == rhs.get();
}

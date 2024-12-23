#include "coContext/coroutine/Coroutine.hpp"

#include "coContext/coroutine/BasePromise.hpp"

#include <utility>

coContext::internal::Coroutine::Coroutine(const Handle handle) noexcept : handle{handle} {}

coContext::internal::Coroutine::Coroutine(Coroutine &&other) noexcept : handle{std::exchange(other.handle, nullptr)} {}

auto coContext::internal::Coroutine::operator=(Coroutine &&other) noexcept -> Coroutine & {
    if (this == std::addressof(other)) return *this;

    this->destroy();

    this->handle = std::exchange(other.handle, nullptr);

    return *this;
}

coContext::internal::Coroutine::~Coroutine() { this->destroy(); }

auto coContext::internal::Coroutine::swap(Coroutine &other) noexcept -> void { std::swap(this->handle, other.handle); }

auto coContext::internal::Coroutine::get() const noexcept -> Handle { return this->handle; }

coContext::internal::Coroutine::operator bool() const noexcept { return static_cast<bool>(this->handle); }

auto coContext::internal::Coroutine::promise() const -> BasePromise & { return this->handle.promise(); }

auto coContext::internal::Coroutine::operator()() const -> void { this->handle(); }

auto coContext::internal::Coroutine::done() const noexcept -> bool { return this->handle.done(); }

auto coContext::internal::Coroutine::destroy() const -> void {
    if (static_cast<bool>(this->handle)) this->handle.destroy();
}

#include "coContext/coroutine/Coroutine.hpp"

#include "coContext/coroutine/BasePromise.hpp"

#include <utility>

coContext::Coroutine::Coroutine(const Handle handle) noexcept : handle{handle} {}

coContext::Coroutine::Coroutine(Coroutine &&other) noexcept :
    handle{std::exchange(other.handle, noOperationCoroutineHandle())} {}

auto coContext::Coroutine::operator=(Coroutine &&other) noexcept -> Coroutine & {
    if (this == std::addressof(other)) return *this;

    this->handle.destroy();

    this->handle = std::exchange(other.handle, noOperationCoroutineHandle());

    return *this;
}

coContext::Coroutine::~Coroutine() { this->handle.destroy(); }

auto coContext::Coroutine::swap(Coroutine &other) noexcept -> void { std::swap(this->handle, other.handle); }

auto coContext::Coroutine::getHandle() const noexcept -> Handle { return this->handle; }

coContext::Coroutine::operator bool() const noexcept {
    return static_cast<bool>(this->handle) && this->handle != noOperationCoroutineHandle();
}

auto coContext::Coroutine::promise() const -> BasePromise & { return this->handle.promise(); }

auto coContext::Coroutine::operator()() const -> void { this->handle(); }

auto coContext::Coroutine::done() const noexcept -> bool { return this->handle.done(); }

auto coContext::Coroutine::noOperationCoroutineHandle() noexcept -> Handle {
    return Handle::from_address(std::noop_coroutine().address());
}

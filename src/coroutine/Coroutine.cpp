#include "coContext/coroutine/Coroutine.hpp"

#include <utility>

coContext::Coroutine::Coroutine(const std::coroutine_handle<> handle) noexcept : handle{handle} {}

coContext::Coroutine::Coroutine(Coroutine &&other) noexcept : handle{std::exchange(other.handle, nullptr)} {}

auto coContext::Coroutine::operator=(Coroutine &&other) noexcept -> Coroutine & {
    if (this == std::addressof(other)) return *this;

    this->destroy();

    this->handle = std::exchange(other.handle, nullptr);

    return *this;
}

coContext::Coroutine::~Coroutine() { this->destroy(); }

auto coContext::Coroutine::swap(Coroutine &other) noexcept -> void { std::swap(this->handle, other.handle); }

auto coContext::Coroutine::get() const noexcept -> std::coroutine_handle<> { return this->handle; }

auto coContext::Coroutine::operator()() const -> void { this->handle(); }

auto coContext::Coroutine::done() const noexcept -> bool { return this->handle.done(); }

auto coContext::Coroutine::destroy() const -> void {
    if (static_cast<bool>(this->handle)) this->handle.destroy();
}

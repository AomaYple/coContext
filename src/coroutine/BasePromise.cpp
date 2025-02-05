#include "coContext/coroutine/BasePromise.hpp"

auto coContext::internal::BasePromise::operator new(const std::size_t bytes) -> void * {
    return getUnsyncMemoryResource()->allocate(bytes);
}

auto coContext::internal::BasePromise::operator delete(void *const pointer, const std::size_t bytes) noexcept -> void {
    getUnsyncMemoryResource()->deallocate(pointer, bytes);
}

auto coContext::internal::BasePromise::swap(BasePromise &other) noexcept -> void {
    std::swap(this->result, other.result);
    std::swap(this->flags, other.flags);
    std::swap(this->exception, other.exception);
    std::swap(this->parentCoroutineId, other.parentCoroutineId);
    std::swap(this->childCoroutine, other.childCoroutine);
}

auto coContext::internal::BasePromise::getResult() const noexcept -> std::int32_t { return this->result; }

auto coContext::internal::BasePromise::setResult(const std::int32_t result) noexcept -> void { this->result = result; }

auto coContext::internal::BasePromise::getFlags() const noexcept -> std::uint32_t { return this->flags; }

auto coContext::internal::BasePromise::setFlags(const std::uint32_t flags) noexcept -> void { this->flags = flags; }

auto coContext::internal::BasePromise::getException() const noexcept -> const std::shared_ptr<std::exception_ptr> & {
    return this->exception;
}

auto coContext::internal::BasePromise::getParentCoroutineId() const noexcept -> std::uint64_t {
    return this->parentCoroutineId;
}

auto coContext::internal::BasePromise::setParentCoroutineId(const std::uint64_t id) noexcept -> void {
    this->parentCoroutineId = id;
}

auto coContext::internal::BasePromise::getChildCoroutine() noexcept -> Coroutine & { return this->childCoroutine; }

auto coContext::internal::BasePromise::setChildCoroutine(Coroutine coroutine) noexcept -> void {
    this->childCoroutine = std::move(coroutine);
}

auto coContext::internal::BasePromise::initial_suspend() const noexcept -> std::suspend_always { return {}; }

auto coContext::internal::BasePromise::final_suspend() const noexcept -> std::suspend_always { return {}; }

auto coContext::internal::BasePromise::unhandled_exception() const noexcept -> void {
    *this->exception = std::current_exception();
}

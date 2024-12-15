#include "coContext/coroutine/BasePromise.hpp"

#include "../memory/memoryResource.hpp"

auto coContext::BasePromise::operator new(const std::size_t numberOfBytes) -> void * {
    return allocator.allocate_bytes(numberOfBytes);
}

auto coContext::BasePromise::operator delete(void *const pointer, const std::size_t numberOfBytes) noexcept -> void {
    allocator.deallocate_bytes(pointer, numberOfBytes);
}

auto coContext::BasePromise::swap(BasePromise &other) noexcept -> void {
    std::swap(this->result, other.result);
    std::swap(this->parentCoroutineIdentity, other.parentCoroutineIdentity);
    std::swap(this->childCoroutine, other.childCoroutine);
}

auto coContext::BasePromise::getResult() -> std::future<std::int32_t> { return this->result.get_future(); }

auto coContext::BasePromise::setResult(const std::int32_t result) -> void {
    this->result = std::promise<std::int32_t>{};
    this->result.set_value(result);
}

auto coContext::BasePromise::getParentCoroutineIdentity() const noexcept -> std::uint64_t {
    return this->parentCoroutineIdentity;
}

auto coContext::BasePromise::setParentCoroutineIdentity(const std::uint64_t identity) noexcept -> void {
    this->parentCoroutineIdentity = identity;
}

auto coContext::BasePromise::getChildCoroutine() noexcept -> Coroutine & { return this->childCoroutine; }

auto coContext::BasePromise::setChildCoroutine(Coroutine &&coroutine) noexcept -> void {
    this->childCoroutine = std::move(coroutine);
}

auto coContext::BasePromise::initial_suspend() const noexcept -> std::suspend_always { return {}; }

auto coContext::BasePromise::final_suspend() const noexcept -> std::suspend_always { return {}; }

auto coContext::BasePromise::unhandled_exception() const -> void { throw; }

thread_local std::pmr::polymorphic_allocator<> coContext::BasePromise::allocator{getMemoryResource()};

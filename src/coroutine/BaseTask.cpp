#include "coContext/coroutine/BaseTask.hpp"

#include "coContext/coroutine/BasePromise.hpp"

auto coContext::internal::BaseTask::swap(BaseTask &other) noexcept -> void {
    std::swap(this->coroutine, other.coroutine);
    std::swap(this->exception, other.exception);
}

auto coContext::internal::BaseTask::getCoroutine() noexcept -> Coroutine & { return this->coroutine; }

auto coContext::internal::BaseTask::throwException() const -> void {
    if (const std::exception_ptr exception{*this->exception}; static_cast<bool>(exception))
        std::rethrow_exception(exception);
}

auto coContext::internal::BaseTask::await_ready() const noexcept -> bool { return {}; }

auto coContext::internal::BaseTask::await_suspend(const std::coroutine_handle<> genericCoroutineHandle) -> void {
    const auto parentCoroutineHandle{Coroutine::Handle::from_address(genericCoroutineHandle.address())};

    this->coroutine.getPromise().setParentCoroutineId(std::hash<Coroutine::Handle>{}(parentCoroutineHandle));
    parentCoroutineHandle.promise().setChildCoroutine(std::move(this->coroutine));
}

coContext::internal::BaseTask::BaseTask(Coroutine coroutine, std::shared_ptr<std::exception_ptr> exception) noexcept :
    coroutine{std::move(coroutine)}, exception{std::move(exception)} {}

#include "coContext/coroutine/GenericTask.hpp"

#include <utility>

coContext::GenericTask::GenericTask(Coroutine &&coroutine) noexcept : coroutine{std::move(coroutine)} {}

auto coContext::GenericTask::swap(GenericTask &other) noexcept -> void {
    std::swap(this->coroutine, other.coroutine);
    std::swap(this->result, other.result);
}

auto coContext::GenericTask::getCoroutine() const noexcept -> const Coroutine & { return this->coroutine; }

auto coContext::GenericTask::getResult() const noexcept -> std::int32_t { return this->result; }

auto coContext::GenericTask::setResult(const std::int32_t result) noexcept -> void { this->result = result; }

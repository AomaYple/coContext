#include "coContext/coroutine/Task.hpp"

auto coContext::operator==(const Task<>::Promise lhs, const Task<>::Promise rhs) noexcept -> bool {
    const auto baseLhs{static_cast<BasePromise>(lhs)}, baseRhs{static_cast<BasePromise>(rhs)};

    return baseLhs == baseRhs;
}

auto coContext::operator==(const Task<> lhs, const Task<> rhs) noexcept -> bool {
    return lhs.getCoroutine() == rhs.getCoroutine();
}

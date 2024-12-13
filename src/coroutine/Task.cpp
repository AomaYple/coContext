#include "coContext/coroutine/Task.hpp"

auto coContext::operator==(const Task<>::Promise &lhs, const Task<>::Promise &rhs) noexcept -> bool {
    const auto &baseLhs{static_cast<const BasePromise &>(lhs)}, baseRhs{static_cast<const BasePromise &>(rhs)};

    return baseLhs == baseRhs;
}

auto coContext::operator==(const Task<> lhs, const Task<> rhs) noexcept -> bool {
    const auto baseLhs{static_cast<BaseTask>(lhs)}, baseRhs{static_cast<BaseTask>(rhs)};

    return baseLhs == baseRhs;
}

#include <coContext/coContext.hpp>
#include <print>

using namespace std::chrono_literals;

[[nodiscard]] auto function() -> coContext::Task<> { std::println("sleep result: {}", co_await coContext::sleep(2s)); }

[[nodiscard]] auto cancelFunction(const std::uint64_t taskId) -> coContext::Task<> {
    std::println("cancel result: {}", co_await coContext::cancel(taskId));
}

[[nodiscard]] auto main() -> int {
    const auto [value, taskId]{spawn(function)};
    spawn(cancelFunction, taskId);

    coContext::run();
}

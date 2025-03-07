#include <coContext/coContext.hpp>
#include <print>

using namespace std::chrono_literals;

[[nodiscard]] auto function() -> coContext::Task<> { std::println("sleep result: {}", co_await coContext::sleep(2s)); }

[[nodiscard]] auto cancelFunction() -> coContext::Task<> {
    std::println("cancel result: {}", co_await coContext::cancelAny());
}

[[nodiscard]] auto main() -> int {
    spawn(function);
    spawn(function);
    spawn(cancelFunction);

    coContext::run();
}

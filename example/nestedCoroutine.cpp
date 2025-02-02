#include <coContext/coContext.hpp>
#include <print>

using namespace std::string_view_literals;

[[nodiscard]] auto nestedFunction() -> coContext::Task<> { co_return; }

[[nodiscard]] auto function() -> coContext::Task<> {
    std::println("nestedFunction calling"sv);

    co_await nestedFunction();

    std::println("nestedFunction returned"sv);
}

auto main() -> int {
    spawn(function);

    coContext::run();
}

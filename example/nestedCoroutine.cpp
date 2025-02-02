#include <coContext/coContext.hpp>
#include <print>

using namespace std::string_view_literals;

[[nodiscard]] auto functionA() -> coContext::Task<> { co_return; }

[[nodiscard]] auto functionB() -> coContext::Task<std::int32_t> { co_return 1; }

[[nodiscard]] auto function() -> coContext::Task<> {
    co_await functionA();

    std::int32_t result{co_await functionB()};

    result += co_await coContext::noOperation();

    std::println("{}"sv, result);
}

auto main() -> int {
    spawn(function);

    coContext::run();
}

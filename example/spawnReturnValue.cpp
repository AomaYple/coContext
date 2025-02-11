#include <coContext/coContext.hpp>
#include <print>

using namespace std::string_view_literals;

[[nodiscard]] auto function() -> coContext::Task<std::int32_t> { co_return co_await coContext::noOperation(); }

[[nodiscard]] auto main() -> int {
    auto [value, taskId]{spawn<std::int32_t>(function)};
    const std::jthread worker{[&value] { std::println("spawn result: {}"sv, value.get()); }};

    coContext::run();
}

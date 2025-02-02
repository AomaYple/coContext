#include <coContext/coContext.hpp>
#include <print>

using namespace std::string_view_literals;

[[nodiscard]] auto function() -> coContext::Task<std::int32_t> { co_return co_await coContext::noOperation(); }

auto main() -> int {
    coContext::SpawnResult result{spawn<std::int32_t>(function)};
    const std::jthread worker{[&result] { std::println("{}"sv, result.value.get()); }};

    coContext::run();
}

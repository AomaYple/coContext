#include <coContext/coContext.hpp>
#include <print>

using namespace std::chrono_literals;

[[nodiscard]] auto timer() -> coContext::Task<> { std::println("{}", co_await coContext::sleep(1s, 50ns)); }

auto main() -> int {
    spawn(timer);

    coContext::run();
}

#include <coContext/coContext.hpp>
#include <print>

[[nodiscard]] auto function() -> coContext::Task<> {
    std::println("thread id: {} result: {}", std::this_thread::get_id(), co_await coContext::noOperation());
}

constexpr auto execute() {
    spawn(function);

    coContext::run();
}

[[nodiscard]] auto main() -> int {
    std::vector<std::jthread> workers;
    for (std::uint32_t i{}; i != std::thread::hardware_concurrency() - 1; ++i) workers.emplace_back(execute);

    execute();
}

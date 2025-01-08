#include <coContext/coContext.hpp>

[[nodiscard]] auto func() -> coContext::Task<> { co_return; }    // 简短的协程函数

constexpr auto execute() {
    spawn(func);

    coContext::run();
}    // 添加协程并运行

auto main() -> int {
    std::vector<std::jthread> workers;
    for (std::uint8_t i{}; i != std::thread::hardware_concurrency() - 1; ++i)
        workers.emplace_back(execute);    // 循环创建线程

    execute();
}
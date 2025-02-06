#include <coContext/coContext.hpp>

[[nodiscard]] auto func() -> coContext::Task<> {    // Task模板参数为<>，表示该协程不返回任何值
    co_await coContext::noOperation();    // 发起close操作
    // co_return; 无返回值的协程可以省略co_return语句
}

auto main() -> int {
    spawn(func);    // 将func函数作为协程任务加入到协程调度器中

    coContext::run();    // 启动调度器
}

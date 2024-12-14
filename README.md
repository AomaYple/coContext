## 介绍

本项目是Linux上一个基于**协程**和**io_uring**的C++高并发异步框架，让你可以像写同步代码一样实现高并发异步编程。

## 特性

- 支持嵌套**任意数量**的**任意返回值**的协程
- 支持多线程
- 支持绝大部分的io_uring支持的系统调用`read` `write` `send` `recv` `accept`等
- 支持纳秒级别的定时器`sleep(1ns)` `sleep(1s)`
- 支持IO超时`timeout(accpet, 1s)`
- 支持IO取消`cancel(taskIdentify)` `cancel(fileDescriptor)` `cancelAny()`

## 用法

仅需引入`coContext.hpp`头文件，即可使用所有功能

```cpp
#include <coContext/coContext.hpp> 
```

异步发起close系统调用，`Task`类模板参数为`<>`代表当前协程无返回值，使用`spawn`添加协程，使用`coContext::run()`启动调度器

```cpp
#include <coContext/coContext.hpp>

[[nodiscard]] auto func() -> coContext::Task<> { co_await coContext::close(-1); }

auto main() -> int {
    spawn(func);
    coContext::run();
}
```

任意嵌套任意返回值的协程，协程可以通过`coContext::spawn()`添加，也可以通过`co_await`关键字等同步调用

```cpp
#include <coContext/coContext.hpp>
#include <print>
#include <thread>

[[nodiscard]] auto funcA() -> coContext::Task<std::int32_t> { co_return 1; }

[[nodiscard]] auto funcB() -> coContext::Task<std::int32_t> {
    std::int32_t result{co_await coContext::close(-1)};

    result += co_await funcA() + co_await funcA();

    co_return result;
}

[[nodiscard]] auto funcD() -> coContext::Task<> { co_await coContext::close(-1); }

[[nodiscard]] auto func() -> coContext::Task<std::int32_t> {
    std::int32_t result{co_await coContext::close(-1)};

    result += co_await funcB();

    co_await funcD();

    co_return result;
}

auto main() -> int {
    coContext::SpawnResult result{spawn<std::int32_t>(func)};

    const std::jthread worker{[&result] { std::println("{}", result.result.get()); }};

    coContext::run();
}
```

## 依赖

- [Linux内核](https://www.kernel.org) >= 6.11
- [GCC](https://gcc.gnu.org) >= 14
- [CMake](https://cmake.org) >= 3.30
- [Ninja](https://ninja-build.org) >= 1.8.2
- [liburing](https://github.com/axboe/liburing) >= 2.7

## 构建

```shell
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release --install-prefix=your_absolute_path
cd build
ninja
```

## 安装

```shell
ninja install
```

## 导入

```cmake
find_package(coContext REQUIRED)
target_link_libraries(your_target
        PRIVATE
        coContext
)
```

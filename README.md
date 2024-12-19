## 介绍

本项目是Linux上一个基于**协程**和**io_uring**的C++高并发异步框架，让你可以像写同步代码一样实现高并发异步编程。

## 特性

- 支持io_uring支持的所有系统调用`read` `write` `send` `recv` `accept`等
- 支持纳秒级别的定时器`sleep(1s, 1ns)`
- 支持IO超时`recv(socketFileDescriptor, buffer, 0) | timeout(1s)`
- 支持IO取消`cancel(taskIdentify)` `cancel(fileDescriptor)` `cancelAny()`
- 支持嵌套**任意数量**的**任意返回值**的协程
- 支持多线程

## 基础用法

仅需引入`coContext.hpp`头文件，即可使用所有功能

```c++
#include <coContext/coContext.hpp> 
```

简单示例，异步发起close系统调用

```c++
#include <coContext/coContext.hpp>

[[nodiscard]] auto func() -> coContext::Task<> {    // Task模板参数为<>，表示该协程不返回任何值
    co_await coContext::close(-1);                  // 发起close操作
    // co_return; 无返回值的协程可以省略co_return语句
}

auto main() -> int {
    spawn(func);         // 将func函数作为协程任务加入到协程调度器中

    coContext::run();    //启动调度器
}
```

## 依赖

- [Linux内核](https://www.kernel.org) >= 6.12
- [GCC](https://gcc.gnu.org) >= 14
- [CMake](https://cmake.org) >= 3.30
- [Ninja](https://ninja-build.org) >= 1.8.2
- [liburing](https://github.com/axboe/liburing) >= 2.7
- [mimalloc](https://github.com/microsoft/mimalloc)

建议使用[Arch Linux](https://archlinux.org)

## 构建

```shell
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release --install-prefix=your_absolute_path
cd build
ninja
```

额外CMake选项

- `-DCO_CONTEXT_NATIVE=ON` 启用本机指令集（`-march=native`）

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

## 更多示例

<details>
<summary>每秒触发的定时器</summary>

```c++
[[nodiscard]] auto func() -> coContext::Task<> {
    while (true) {
        co_await coContext::sleep(1s);

        std::println("Hello, coContext!");
    }
}
```

</details>
<details>
<summary>IO超时控制</summary>

```c++
[[nodiscard]] auto func(const std::int32_t socketFileDescriptor) -> coContext::Task<> {
    std::vector<std::byte> buffer{1024};
    const std::int32_t result{
        co_await (coContext::receive(socketFileDescriptor, buffer, 0) | coContext::timeout(3s))};    // 限时3秒

    std::println("received: {}", result);
}
```

</details>
<details>
<summary>IO取消</summary>

- 基于taskIdentity取消任务中正在运行的io

```c++
[[nodiscard]] auto func() -> coContext::Task<> { co_await coContext::sleep(4s); }    // 发起一个4s的定时

[[nodiscard]] auto cancelFunc(const std::uint64_t taskIdentity) -> coContext::Task<> {
    co_await coContext::cancel(taskIdentity);    // 基于任务标识符取消任务中正在运行的io
}

auto main() -> int {
    const std::uint64_t taskIdentity{spawn(func)};
    spawn(cancelFunc, taskIdentity);

    coContext::run();
}
```

- 基于文件描述符取消io

```c++
[[nodiscard]] auto cancelFunc(const std::int32_t socketFileDescriptor) -> coContext::Task<> {
    co_await coContext::cancel(socketFileDescriptor,
                               true);    // 第二个参数为 true 时，会取消该文件描述符上的所有io，否则只取消第一个io
}

[[nodiscard]] auto func(const std::int32_t socketFileDescriptor) -> coContext::Task<> {
    spawn(cancelFunc, socketFileDescriptor);

    std::vector<std::byte> buffer{1024};
    co_await coContext::receive(socketFileDescriptor, buffer, 0);
}
```

- 取消所有IO

```c++
[[nodiscard]] auto func() -> coContext::Task<> { co_await coContext::cancelAny(); }
```

并且，支持同步取消版本`syncCancel`，用法与`cancel`相同
</details>
<details>
<summary>任意嵌套任意返回值的协程</summary>

```c++
#include <coContext/coContext.hpp>
#include <print>
#include <thread>

using namespace std::chrono_literals;

[[nodiscard]] auto funcA() -> coContext::Task<std::int32_t> { co_return 1; }    // 该协程什么也不做，直接返回1

[[nodiscard]] auto funcB() -> coContext::Task<std::int32_t> {
    std::int32_t result{co_await coContext::close(-1)};    // 发起close请求

    result += co_await funcA();    // 将result加上funcA的返回值

    co_return result;    // 返回result
}

[[nodiscard]] auto funcD() -> coContext::Task<> {
    co_await coContext::close(-1);
}    // 该协程发起close请求，但不返回任何值

[[nodiscard]] auto func() -> coContext::Task<std::int32_t> {
    std::int32_t result{co_await coContext::close(-1)};    // 发起close请求

    result += co_await funcB();    // 将result加上funcB的返回值

    co_await funcD();    // 调用funcD

    co_return result;    // 返回result
}

auto main() -> int {
    coContext::SpawnResult result{spawn<std::int32_t>(func)};    // 添加func，并以SpawnResult类型保存返回值

    const std::jthread worker{[&result] {
        std::println("{}", result.result.get());    // 在新线程中输出result的值
    }};

    coContext::run();
}
```

</details>
<details>
<summary>多线程</summary>

```c++
#include <coContext/coContext.hpp>
#include <thread>

[[nodiscard]] auto func() -> coContext::Task<> { co_await coContext::close(-1); }

auto main() -> int {
    std::vector<std::jthread> workers;
    for (std::uint8_t i{}; i != std::thread::hardware_concurrency() - 1; ++i) {
        workers.emplace_back([] {
            spawn(func);
            coContext::run();
        });
    }

    spawn(func);
    coContext::run();
}
```

</details>

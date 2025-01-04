## 介绍

本项目是Linux上一个基于**协程**和**io_uring**的异步高并发库，让你可以以**同步**的方式实现**异步**逻辑，轻松实现高性能的高并发程序

## 特性

- io_uring支持的所有系统调用`accept` `send` `recv` `read` `write`等
- 纳秒级别的定时器`sleep(1s, 1ns)`
- IO超时`recv(socketFileDescriptor, buffer, 0) | timeout(1s)`
- IO取消`cancel(taskIdentify)` `cancel(fileDescriptor)` `cancelAny()`
- 嵌套**任意数量**的**任意返回值**的协程
- 多线程
- 直接文件描述符，可以与普通文件描述符相互转换
- 支持多发射IO

## 基础用法

仅需引入`coContext.hpp`头文件，即可使用所有功能

```c++
#include <coContext/coContext.hpp> 
```

简单示例，同步地执行异步的close系统调用

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
- [liburing](https://github.com/axboe/liburing) >= 2.8
- [mimalloc](https://github.com/microsoft/mimalloc)

建议使用[Arch Linux](https://archlinux.org)

## 构建

```shell
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release --install-prefix=your_absolute_path
cd build
ninja
```

额外CMake选项

- `-DCO_CONTEXT_NATIVE=ON` 启用本机指令集（只在`Release`下生效）

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

### 启动和停止

`run()`启动当前线程的调度器，`stop()`停止当前线程的调度器

### 如何使用`spawn`的返回值

```c++
#include <coContext/coContext.hpp>
#include <print>
#include <thread>

using namespace std::string_view_literals;

[[nodiscard]] auto func() -> coContext::Task<std::int32_t> {
    co_return co_await coContext::close(-1);
}    // 发起close调用，并返回一个std::int32_t类型的值

auto main() -> int {
    coContext::SpawnResult result{coContext::spawn<std::int32_t>(
        func)};    // SpawnResult类型有两个成员，一个为std::future<T>类型的任务返回值，这里T为std::int32_t，另一个为std::uint64_t类型的任务标识符

    const std::jthread worker{[&result] {
        std::println("{}"sv, result.value.get());    // 阻塞地等待任务结束，并输出任务返回值
        std::println("{}"sv, result.taskIdentity);    // 输出任务标识符
    }};

    coContext::run();
}
```

<details>
<summary>定时器</summary>

```c++
[[nodiscard]] auto func() -> coContext::Task<> {
    std::println("{}", co_await coContext::sleep(1s));
}    // 打印1秒定时的结果
```

</details>
<details>
<summary>IO超时控制</summary>

```c++
[[nodiscard]] auto func(const std::int32_t socketFileDescriptor) -> coContext::Task<> {
    std::vector<std::byte> buffer{1024};
    const std::int32_t result{
        co_await (coContext::receive(socketFileDescriptor, buffer, 0) | coContext::timeout(3s))};    // 限时3秒

    std::println("received: {}"sv, result);    // 打印接收到的字节数
}
```

</details>
<details>
<summary>取消IO</summary>

- 基于`taskIdentity`取消任务中正在运行的io

```c++
[[nodiscard]] auto func() -> coContext::Task<> { co_await coContext::sleep(4s); }    // 发起一个4s的定时

[[nodiscard]] auto cancelFunc(const std::uint64_t taskIdentity) -> coContext::Task<> {
    co_await coContext::cancel(taskIdentity);    // 基于任务标识符取消任务中正在运行的io
}
```

- 基于文件描述符取消io

```c++
[[nodiscard]] auto cancelFunc(const std::int32_t socketFileDescriptor) -> coContext::Task<> {
    co_await coContext::cancel(
        socketFileDescriptor,
        true);    // 第二个参数为默认为false，为true时，会取消该文件描述符上的所有io，否则只取消第一个io
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
[[nodiscard]] auto funcA() -> coContext::Task<std::int32_t> {    // 返回值为std::int32_t类型
    const std::int32_t result{co_await coContext::close(-1)};    // 发起close调用

    co_return result + 3;    // 返回result + 3
}

[[nodiscard]] auto func() -> coContext::Task<> {
    std::int32_t result{co_await funcA()};    // 调用funcA并等待返回值
    result += co_await funcA();    // 再次调用funcA并等待返回值

    std::println("{}"sv, result);    // 打印result
}
```

</details>
<details>
<summary>多线程</summary>

```c++
#include <coContext/coContext.hpp>
#include <thread>

[[nodiscard]] auto func() -> coContext::Task<> { co_await coContext::close(-1); }    // 简短的协程函数

auto main() -> int {
    std::vector<std::jthread> workers;
    for (std::uint8_t i{}; i != std::thread::hardware_concurrency() - 1; ++i) {    // 循环创建线程
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

<details>
<summary>直接文件描述符</summary>

- 优点
    - 直接文件描述符的操作开销较普通文件描述符更低，因为内核在操作开始时会抓取文件描述符引用计数，并在操作完成后丢弃它
    - 如果进程文件表是共享的（例如多线程程序），普通文件描述符的开销会更大

```c++
[[nodiscard]] auto func() -> coContext::Task<> {
    const std::int32_t directFileDescriptor{
        co_await coContext::openDirect("file"sv, O_RDONLY)};    // 以只读方式打开"file"文件, 并返回直接文件描述符
    std::println("open direct result: {}"sv, directFileDescriptor);    // 输出打开文件结果

    std::vector<std::byte> buffer{1024};
    const std::int32_t result{
        co_await (coContext::read(directFileDescriptor, buffer) |
                  coContext::direct())};    // 使用"coContext::direct()"标记以直接文件描述符方式读取文件

    std::println("read result: {}"sv, result);    // 输出读取结果
}
```

- 直接文件描述符必须以`closeDirect()`关闭
- 直接文件描述的IO操作必须以`direct()`标记
- 直接文件描述符可以通过`installDirect()`转换为普通文件描述符，普通文件描述符可以通过`toDirect()`转换为直接文件描述符
- 可以通过`directSocket` `acceptDirect` `multipleAcceptDirect` `openDirect`获得直接文件描述符
- 转换后的直接文件描述符和普通文件描述符**相互独立**

</details>

<details>

<summary>多发射IO</summary>

### 什么是多发射IO？就是只需发出一次请求，就可以多次接收到结果

- 优点
    - 减少了系统调用的次数
    - 减少了内核态和用户态的切换次数
    - 减少了内核态和用户态的数据拷贝次数
    - 减少了内核态和用户态的上下文切换次数
- 支持`multipleSleep` `multiplePoll` `multipleAccept` `multipleAcceptDirect` `multipleReceive` `multipleRead`

```c++
[[nodiscard]] auto normalClose(const std::int32_t socket) -> coContext::Task<> { co_await coContext::close(socket); }

[[nodiscard]] auto normalSend(const std::int32_t socket, const std::span<const std::byte> data) -> coContext::Task<> {
    if (const std::int32_t result{co_await coContext::send(socket, data, 0)}; result <= 0) spawn(normalClose, socket);
}

[[nodiscard]] constexpr auto receiveAction(const std::int32_t socket, const std::int32_t result,
                                           const std::span<const std::byte> data) {
    if (result > 0) spawn(normalSend, socket, data);    // 如果result大于0，就调用normalSend协程，将data作为参数传入
    else spawn(normalClose, socket);    // 否则调用normalClose协程，将socket作为参数传入
}

constexpr auto acceptAction(const std::int32_t socket, const std::int32_t result) {
    if (result >= 0) {
        spawn(
            coContext::multipleReceive,
            [result](const std::int32_t receiveResult, const std::span<const std::byte> data) {
                receiveAction(result, receiveResult, data);
            },
            result, 0, coContext::none());
        // 如果result大于等于0，就调用multipleReceive协程，将receiveAction作为回调函数传入，并利用lambda捕获result
        // coContext::none()表示不使用标记
        // 如果使用coContext::direct()则表示使用直接IO，使用coContext::timeout()则表示使用超时，并且可以组合使用
    } else spawn(normalClose, socket);
}

[[nodiscard]] auto multipleAccept(const std::int32_t socket) -> coContext::Task<> {
    co_await coContext::multipleAccept([socket](const std::int32_t result) { acceptAction(socket, result); }, socket,
                                       nullptr, nullptr, 0);
    // 阻塞地执行multipleAccept协程，将acceptAction作为回调函数传入，并利用lambda捕获socket
}
```

</details>
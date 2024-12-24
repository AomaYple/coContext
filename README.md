## 介绍

本项目是Linux上一个基于**协程**和**io_uring**的异步高并发库，让你可以以**同步**的方式实现**异步**逻辑，轻松实现高性能的高并发程序

## 特性

- io_uring支持的所有系统调用`accept` `send` `recv` `read` `write`等
- 纳秒级别的定时器`sleep(1s, 1ns)`
- IO超时`recv(socketFileDescriptor, buffer, 0) | timeout(1s)`
- IO取消`cancel(taskIdentify)` `cancel(fileDescriptor)` `cancelAny()`
- 嵌套**任意数量**的**任意返回值**的协程
- 多线程
- 直接文件描述符，以`directSocket` `acceptDirect` `openDirect`方式获得

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

## 内存管理

- 基于C++17`std::pmr`进行内存管理，各个线程都有自己的内存池，即`thread_local`
- 内存池实现为`std::pmr::unsynchronized_pool_resource`，内存池内存不足时会向`mimalloc`申请内存
- 标准库`coroutine`使用默认的内存分配方式，所以在类范围重载了`operator new`和`operator delete`，使用内存池分配和释放内存
- 协程和所有STL容器都使用内存池，减少了内存碎片和使内存分布更加紧凑，极大地提升了**缓存友好性**
- 内存分配**不是侵入式**的，不会影响到除了本库之外的其他代码

### 除了`SpawnResult.value`能够跨线程使用，其他所有类型都必须在本线程中使用

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
<summary>每秒触发的定时器</summary>

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

- 直接文件描述符的优点
    - 直接文件描述符的操作开销较普通文件描述符更低，因为内核在操作开始时抓取普通文件描述符引用计数，并在操作完成后丢弃它
    - 如果进程文件表是共享的（例如多线程程序），开销会更大

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
- 直接文件描述符可以通过`installDirect()`转换为普通文件描述符
- 暂不支持普通文件描述符转换为直接文件描述符
- 转换后的直接文件描述符和普通文件描述符**相互独立**

</details>

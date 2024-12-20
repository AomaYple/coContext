## 介绍

本项目是Linux上一个基于**协程**和**io_uring**的C++高并发异步框架，让你可以像写同步代码一样实现高并发异步编程

## 特性

- 支持io_uring支持的所有系统调用`read` `write` `send` `recv` `accept`等
- 支持纳秒级别的定时器`sleep(1s, 1ns)`
- 支持IO超时`recv(socketFileDescriptor, buffer, 0) | timeout(1s)`
- 支持IO取消`cancel(taskIdentify)` `cancel(fileDescriptor)` `cancelAny()`
- 支持嵌套**任意数量**的**任意返回值**的协程
- 支持多线程
- 支持直接文件描述符，支持以`socket` `accept` `open`方式获得直接文件描述符

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

## 内存管理

- 考虑到多线程之间数据共享的开销，调度器实例都是`thread_local`的，也就是说每个线程都有自己的调度器实例，且**不能跨线程**使用
- 本框架使用内存池`std::pmr::unsynchronized_pool_resource`进行内存分配， 使内存分布更紧凑，减少内存碎片
- 内存池都是`thread_local`的，减少多线程分配和释放内存时的锁开销
- 内存池的上游是`mimalloc`，如果内存池内存不足，会向`mimalloc`申请内存
- 标准库`coroutine`默认使用全局`operator new`，本项目在类范围重载了`operator new`和`operator delete`，使用内存池分配内存
- 由于本框架的协程和所有STL容器都使用内存池，所以极大地提升了**缓存友好性**
- 内存分配**不是侵入式**的，不会影响到除了本框架之外的其他代码

### 除了`SpawnResult.result`能够跨线程使用，其他所有类型都必须在本线程中使用

## 更多示例

### 启动和停止

`coContext::run()`启动当前线程的调度器，`coContext::stop()`停止当前线程的调度器

### 如何使用`spawn`的返回值

```c++
#include <coContext/coContext.hpp>
#include <print>
#include <thread>

[[nodiscard]] auto func() -> coContext::Task<std::int32_t> {
    co_return co_await coContext::close(-1);
}    // 发起close调用，并返回一个std::int32_t类型的值

auto main() -> int {
    coContext::SpawnResult result{coContext::spawn<std::int32_t>(
        func)};    // SpawnResult类型有两个成员，一个为std::uint64_t类型的任务标识符，一个为std::future<T>类型的任务返回值，这里T为std::int32_t

    const std::jthread worker{[&result] {
        std::println("{}", result.taskIdentity);    // 输出任务标识符
        std::println("{}", result.result.get());    // 阻塞地等待任务结束，并输出任务返回值
    }};

    coContext::run();
}
```

<details>
<summary>每秒触发的定时器</summary>

```c++
[[nodiscard]] auto func() -> coContext::Task<> {
    while (true) {    // 无限循环
        co_await coContext::sleep(1s);    // 等待1秒

        std::println("Hello, coContext!");    // 输出
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

    std::println("{}", result);    // 打印result
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

<details>
<summary>直接文件描述符</summary>

- 直接文件描述符的优点
    - 直接文件描述符的操作开销较普通文件描述符更低，因为内核在操作开始时抓取普通文件描述符引用计数，并在操作完成后丢弃它
    - 如果进程文件表是共享的（例如多线程程序），开销会更大

```c++
[[nodiscard]] auto func() -> coContext::Task<> {
    const std::int32_t directFileDescriptor{
        co_await coContext::openDirect("file"sv, O_RDONLY)};    // 以只读方式打开"file"文件, 并返回直接文件描述符
    std::println("open direct result: {}", directFileDescriptor);

    std::vector<std::byte> buffer{1024};
    const std::int32_t result{
        co_await (coContext::read(directFileDescriptor, buffer) |
                  coContext::useDirectFileDescriptor())};    // 使用"coContext::useDirectFileDescriptor()"标记
                                                             // 以直接文件描述符方式读取文件
    std::println("read result: {}", result);
}
```

- 直接文件描述符必须以`coContext::closeDirect()`关闭
- 直接文件描述的IO操作必须以`coContext::useDirectFileDescriptor()`标记
- 直接文件描述符可以通过`coContext::installDirectFileDescriptor()`转换为普通文件描述符
- 转换后的直接文件描述符和普通文件描述符**相互独立**

</details>

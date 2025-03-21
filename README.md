## 介绍

本项目是`Linux`上一个基于**C++20 协程**和`io_uring`的异步高并发库，让你可以以**同步**的方式实现**异步**逻辑，轻松实现高性能的高并发程序

## 特性

- 绝大部分系统调用，如`read` `write` `send` `recv`等
- 纳秒级别的定时器`sleep(1s, 1ns)`
- IO超时`recv(socketFileDescriptor, buffer, 0) | timeout(1s)`
- IO取消`cancel(taskId)` `cancel(fileDescriptor)` `cancelAny()`
- 嵌套**任意数量**的**任意返回值**的协程
- 多线程
- **异步高性能**且**多级别**的日志系统
- 直接文件描述符，可以与普通文件描述符**相互转换**
- 多发射IO
- **零拷贝**发送
- **百万**级并发，比`Asio`的吞吐量高`388%`

## 基础用法

仅需引入`coContext.hpp`头文件，即可使用所有功能

```c++
#include <coContext/coContext.hpp> 
```

简单示例，向调度器添加`function`协程，启动调度器；`function`协程发起`noOperation`操作

```c++
#include <coContext/coContext.hpp>

[[nodiscard]] auto function() -> coContext::Task<> {    // Task模板参数为<>，表示该协程不返回任何值
    co_await coContext::noOperation();    // 发起noOperation操作
    // co_return; 无返回值的协程可以省略co_return语句
}

auto main() -> int {
    spawn(function);    // 将function函数作为协程任务加入到协程调度器中

    coContext::run();    // 启动调度器
}
```

## 依赖

- 编译
    - [Linux 内核](https://www.kernel.org) >= 6.12
    - [GCC](https://gcc.gnu.org) >= 14
    - [CMake](https://cmake.org) >= 3.30
    - [Ninja](https://ninja-build.org) >= 1.8.2
    - [liburing](https://github.com/axboe/liburing) >= 2.9
    - [mimalloc](https://github.com/microsoft/mimalloc) >= 1.0.0
    - [Asio](https://think-async.com/Asio) >= 1.17.0（可选）
- 运行
    - [Linux 内核](https://www.kernel.org) >= 6.12
    - [GCC](https://gcc.gnu.org) >= 14
    - [liburing](https://github.com/axboe/liburing) >= 2.9
    - [mimalloc](https://github.com/microsoft/mimalloc) >= 1.0.0

建议使用[Arch Linux](https://archlinux.org)

## 构建

```shell
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release --install-prefix=your_absolute_path
cd build
ninja
```

额外CMake选项

- `-DCCACHE=ON` 启用`ccache`加速编译
- `-DNATIVE=ON` 启用本机指令集（构建类型为`Release`时生效）
- `-DBENCHMARK=ON` 启用性能测试

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

## 性能

环境：

- `16 × 11th Gen Intel® Core™ i7-11800H @ 2.30GHz`
- `2 × 8GB` `DDR4 3200MHz`
- `Arch Linux` `6.13.2-zen1-1-zen (64 位)`
- `gcc (GCC) 14.2.1 20250207`
- `liburing 2.9`
- `mimalloc 2.1.9`
- `Asio 1.30.2`
- `wrk 358c286 [epoll] Copyright (C) 2012 Will Glozer`

测试：  
使用 [wrk](https://github.com/wg/wrk)进行性能测试

- coContext
  [benchmark/coContext.cpp](https://github.com/AomaYple/coContext/blob/main/benchmark/coContext.cpp)
  ```
  ❯ wrk -t $(nproc) -c 1007 http://localhost:8080
  Running 10s test @ http://localhost:8080
    16 threads and 1007 connections
    Thread Stats   Avg      Stdev     Max   +/- Stdev
      Latency   571.64us    3.43ms 233.47ms   98.17%
      Req/Sec   104.55k    43.06k  196.86k    67.78%
    16600241 requests in 10.10s, 601.59MB read
  Requests/sec: 1644235.82
  Transfer/sec:     59.59MB
  ```
- asio（使用`io_uring`作为后端，并且设置`ASIO_CONCURRENCY_HINT_UNSAFE`免去锁的开销）
  [benchmark/asio.cpp](https://github.com/AomaYple/coContext/blob/main/benchmark/asio.cpp)
  ```
  ❯ wrk -t $(nproc) -c 1007 http://localhost:8080
  Running 10s test @ http://localhost:8080
    16 threads and 1007 connections
    Thread Stats   Avg      Stdev     Max   +/- Stdev
      Latency     7.74ms   41.10ms 733.58ms   98.14%
      Req/Sec    21.45k     3.55k   64.27k    95.61%
    3403478 requests in 10.10s, 123.34MB read
  Requests/sec: 336977.45
  Transfer/sec:     12.21MB
  ```

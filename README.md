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
- **百万**级并发，`RPS`比`libevent`高`9.36%`左右

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
    - [mimalloc](https://github.com/microsoft/mimalloc)
    - [libevent](https://libevent.org)（可选）
- 运行
    - [Linux 内核](https://www.kernel.org) >= 6.12
    - [GCC](https://gcc.gnu.org) >= 14
    - [liburing](https://github.com/axboe/liburing) >= 2.9
    - [mimalloc](https://github.com/microsoft/mimalloc)
    - [libevent](https://libevent.org)（可选）

建议使用[Arch Linux](https://archlinux.org)

## 构建

```shell
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release --install-prefix=your_absolute_path
cd build
ninja
```

额外CMake选项

- `-DNATIVE=ON` 启用本机指令集（只在`Release`下生效）
- `-DEXAMPLE=ON` 启用示例
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

## 注意事项

- 用户只能使用`coContext`和`coContext::logger`命名空间下的函数和类
- `coContext::Tast<T>`只能用于声明协程函数的返回值

## 性能

环境：

- `16 × 11th Gen Intel® Core™ i7-11800H @ 2.30GHz`
- `2 × 8GB` `DDR4 3200MHz`
- `Arch Linux` `6.13.1-zen1-1-zen (64 位)`
- `gcc (GCC) 14.2.1 20240910`
- `liburing 2.9`
- `mimalloc 2.1.9`
- `libevent 2.1.12`

测试：  
使用 [wrk](https://github.com/wg/wrk)进行性能测试

- coContext
  [benchmark/coContext.cpp](https://github.com/AomaYple/coContext/blob/main/benchmark/coContext.cpp)
  ```
  ❯ wrk -t $(nproc) -c 950 http://localhost:8080
  Running 10s test @ http://localhost:8080
    16 threads and 950 connections
    Thread Stats   Avg      Stdev     Max   +/- Stdev
      Latency     1.37ms    5.24ms 178.03ms   96.44%
      Req/Sec   102.89k    53.92k  198.21k    58.47%
    16258208 requests in 10.10s, 589.19MB read
  Requests/sec: 1609945.61
  Transfer/sec:     58.34MB
  ```
- libevent
  [benchmark/libevent.cpp](https://github.com/AomaYple/coContext/blob/main/benchmark/libevent.cpp)
  ```
  ❯ wrk -t $(nproc) -c 950 http://localhost:8080
  Running 10s test @ http://localhost:8080
    16 threads and 950 connections
    Thread Stats   Avg      Stdev     Max   +/- Stdev
      Latency   791.73us    0.94ms  17.01ms   86.78%
      Req/Sec    93.05k     8.99k  208.01k    78.04%
    14866255 requests in 10.10s, 538.75MB read
  Requests/sec: 1472208.89
  Transfer/sec:     53.35MB
  ```

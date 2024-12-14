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

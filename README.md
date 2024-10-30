## 介绍

本项目是Linux上一个基于协程和io_uring的C++异步高并发框架

## 依赖

Linux内核6.1及以上，GCC14及以上，CMake，Ninja，liburing2.7及以上

## 编译

```shell 
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cd build
ninja
```

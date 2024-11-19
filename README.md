## 介绍

本项目是Linux上一个基于协程和io_uring的C++高并发异步框架

## 依赖

Linux内核6.12及以上，GCC 14及以上，CMake 3.30及以上，Ninja 1.8.2及以上，liburing 2.8及以上

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

## 使用

```cmake
find_package(coContext REQUIRED)
target_link_libraries(your_target
        PRIVATE
        coContext::coContext
)
```

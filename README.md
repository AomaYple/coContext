## 介绍

本项目是Linux上一个基于协程和io_uring的C++高并发异步框架

## 依赖

[Linux内核](https://www.kernel.org) >= 6.11，[GCC](https://gcc.gnu.org) >= 14，[CMake](https://cmake.org) >=
3.30，[Ninja](https://ninja-build.org) >= 1.8.2，[liburing](https://github.com/axboe/liburing) >= 2.7

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
        coContext
)
```

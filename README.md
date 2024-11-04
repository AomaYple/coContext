## 介绍

本项目是Linux上一个基于协程和io_uring的C++高并发异步框架

## 依赖

Linux内核6.10及以上，GCC14及以上，CMake3.24及以上，Ninja，liburing2.8及以上

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

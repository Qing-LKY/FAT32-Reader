# FAT32-Reader

## 使用说明



## Build

依赖 [libreadline](https://tiswww.case.edu/php/chet/readline/rltop.html)。

```bash
mkdir -p build && cd build
cmake ../
make
```

编译后在 `build/bin` 目录下得到 `fat32-reader`

## Test

单元测试依赖 [check](https://libcheck.github.io/check/)

测试方法：

```bash
mkdir -p build && cd build
cmake ../
make && ctest -V
```

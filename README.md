# FAT32-Reader

## 使用说明

## 功能简介

本项目运行在 Linux 环境下。它提供了一个交互窗口。
用户可以通过输入指令，读取一个装载了 FAT32 文件系统的磁盘映像，解析、查看该磁盘的目录结构，或从中导出文件。

## 操作指南

运行后，会看到提示前缀 `[ImagePath:FilePath]$ `。ImagePath 是现在所加载的磁盘文件名，FilePath 是当前处在磁盘中的位置。

你可以通过输入来与程序交互：

- 输入 `load`，会进入加载磁盘映像的引导
- 输入 `exit`，可以退出程序
- 输入 `ls`，可以显示当前位置下的文件和文件夹
- 输入 `dump`，进入导出文件的引导，可以导出一个当前位置下的文件
- 输入 `cd`，进入切换目录的引导

## 实现原理

程序利用 FAT32 文件系统的结构，通过解析二进制文件的方式来实现对文件系统的操作。

## 部分细节

1. 对于中文文件名，在 Linux 中，会被解析为 UTF-8，且 Linux 在将其解析为长文件名时，采用的是直接按字节拆分后高位补 0 的方式。我们使用的也是这种解析方式，而不是 FAT32 标准中提出的 UTF-16。这会导致从 Windows 下创建的 FAT32 映像的中文目录项出现乱码，而 Linux 下创建的 FAT32 映像则可以正常完美解析

2. 在 load 和 dump 要求输入目标本地文件名时，请使用绝对目录

3. dump 可以保证读取出的文件内容与原本内容没有差异，但文件名和新文件的具体权限控制需要用户自己把握。

4. cd 不仅可以输入绝对路径和相对路径，也支持使用 .. 和 .

## 编译

依赖 [libreadline](https://tiswww.case.edu/php/chet/readline/rltop.html)。

```bash
mkdir -p build && cd build
cmake ../
make
```

编译后在 `build` 下得到 `fat32-reader`

## 测试

单元测试依赖 [check](https://libcheck.github.io/check/)

测试方法：

```bash
mkdir -p build && cd build
cmake ../
make fat_test && ctest -V
```

## 后续计划

本项目为小组的软件安全实验成品。后续可能会考虑进行命令行、交互等的优化，或考虑添加对 UTF-16 的编码支持。

如果在使用过程中发现了 Bug，欢迎在 issue 中提出，感谢大家的支持 QwQ。

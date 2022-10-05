#ifndef _FAT_FILE_H
#define _FAT_FILE_H
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

// 把文件 fd 的 addr 偏移的 siz 个字节写入 buf 中
// 返回 0 表示正常，出错则返回错误信息
static inline int read_offset(int fd, off_t addr, size_t siz, u8 *buf) {
    if(lseek(fd, addr, SEEK_SET) == -1) return errno;
    if(read(fd, buf, siz) == -1) return errno;
    return 0;
}

// 以 oflag 打开 filepath，记录下 fd
// 返回 0 或错误信息
static inline int open_file(int *fd, char *filepath, int oflag) {
    *fd = open(filepath, oflag);
    return *fd == -1 ? errno : 0;
}
#endif //_FAT_FILE_H
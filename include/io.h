#ifndef _FAT_IO_H
#define _FAT_IO_H
#include "file.h"
#include "fat.h"

#define BUF_SIZE 1000000

extern int img_fd;

int load_disk_image(char *imagepath);
int load_DBR_info();
int disp_DBR_info();

// 将 cluster 中的数据读入到 buf 中，需要保证 buf 的可用空间大小大于簇大小
// 出错返回负数
int read_clus(u32 cluster, u8 *buf);

// 接收簇号，找到该簇号指向的下一个簇
// 如果出错，返回CLUS_EMPTY, CLUS_RESERVED 或 CLUS_BAD
u32 next_clus(u32 clus);
#endif //_FAT_IO_H
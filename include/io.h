#ifndef _FAT_IO_H
#define _FAT_IO_H
#include "file.h"

#define BUF_SIZE 1000000

typedef struct fat_superblock_t {
    u16 size_per_sector;
    u8 sectors_per_cluster;
    u16 reserved_sectors_num;
    u8 FATs_num;
    u32 sectors_num;
    u32 sectors_per_FAT;
    u32 root_clus;
} fat_superblock_t;

extern fat_superblock_t fat_superblock;
extern int img_fd;

int load_disk_image(char *imagepath);
int load_DBR_info();
int disp_DBR_info();
#endif //_FAT_IO_H
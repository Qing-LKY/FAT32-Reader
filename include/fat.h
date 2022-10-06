#ifndef _FAT_FAT_H
#define _FAT_FAT_H

#include "file.h"

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

#define CLUS_EMPTY (0x00000000U)
#define CLUS_RESERVED (0x00000001U)
#define CLUS_BAD (0x0FFFFFF7U)

#define CLUS_IS_USED(clus) (clus >= 0x2U && clus <= 0xFFFFFEFU ? 1 : 0)

// 数据区目录项结构
typedef struct fat_entry {
    char *name;
    u8 attr;     /* 从目录项中直接读取的属性 */
    u32 i_first; /* 文件/目录起始簇号 */
    u32 size;
} fat_entry;

// 解析目录项，返回由该目录下所有目录项组成的数组
// 如果出错，返回 NULL
fat_entry *fat_parse_dir(fat_entry *e);

// 将给定目录项的内容读入buf中，buf内存由调用者管理
// 出错时返回负数
int fat_read_file(fat_entry *e, u8 *buf, u32 *size);

#endif //_FAT_FAT_H
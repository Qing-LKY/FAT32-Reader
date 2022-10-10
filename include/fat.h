#ifndef _FAT_FAT_H
#define _FAT_FAT_H

#include "file.h"
#include "uchar.h"

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

#define CLUS_IS_USED(clus) (clus >= 0x2U && clus <= 0x0FFFFEFU ? 1 : 0)
#define CLUS_IS_END(clus) (clus >= 0x0FFFFFF8U ? 1 : 0)

// 数据区目录项结构
typedef struct fat_entry_t {
    char *name; /* 短目录项名 或 长目录项名 */
    int n_len;  /* name 的长度，通过n_len大小可以判断是长文件名还是短文件名 */
    u8 attr;    /* 从目录项中直接读取的属性 */
    union {
        /* 短目录项中文件/目录起始簇号，长目录项中序数项 */
        u32 i_first;
        u8 ordinal;
    };
    u32 size; /* 文件大小 */
} fat_entry_t;

#define ENTRY_SIZE 0x20
#define ENTRY_ATTR_DIR 0x10U
#define ENTRY_ATTR_ARC 0x20U
#define ENTRY_ATTR_LFN 0x0FU

// 解析目录项，将结果存入到entry中
int fat_parse_entry(fat_entry_t *entry, u8 *buf);

// 解析目录，返回由该目录下所有目录项组成的数组
// 如果出错，返回 NULL
// e 为待解析目录项，len 为该目录中含有目录项的个数
fat_entry_t *fat_parse_dir(fat_entry_t *e, int *len);

// 将给定目录项的内容读入buf中，buf内存由调用者管理，size为buf的大小。
// 出错时返回负数，否则返回有效内容的大小。
int fat_read_file(fat_entry_t *e, u8 *buf, int size);

// 将目录项 src 复制给 des。des 必须是已经声明了空间的。
// 主要作用是自动化 name 字符串的深复制。
void entry_copy(fat_entry_t *des, fat_entry_t *src);

// 释放项数组
int free_entry_array(fat_entry_t *arr, int n);

// 把 e 对应的文件写入 target_fd 中
int fat_to_file(fat_entry_t *e, int target_fd);

#endif //_FAT_FAT_H
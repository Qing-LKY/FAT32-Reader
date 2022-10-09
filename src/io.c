#include "fat.h"
#include "file.h"
#include <string.h>
#include <malloc.h>
#include "io.h"

fat_superblock_t fat_superblock;
u8 img_buf[BUF_SIZE];

int img_fd;

static int fat_offset;  /* FAT1 字节偏移 */
static int data_offset; /* 数据区字节偏移 */

int load_disk_image(char *imagepath) {
    return open_file(&img_fd, imagepath, O_RDWR);
}

int load_DBR_info() {
    static int err;
    fat_superblock_t *sb = &fat_superblock;
    err = read_offset(img_fd, 0, 0x200, img_buf);
    if (err) return err;
    sb->size_per_sector = *(u16 *)(img_buf + 0x0B);
    sb->sectors_per_cluster = *(u8 *)(img_buf + 0x0D);
    sb->reserved_sectors_num = *(u16 *)(img_buf + 0x0E);
    sb->FATs_num = *(u8 *)(img_buf + 0x10);
    sb->sectors_num = *(u32 *)(img_buf + 0x20);
    sb->sectors_per_FAT = *(u32 *)(img_buf + 0x24);
    sb->root_clus = *(u32 *)(img_buf + 0x2C);
    fat_offset = sb->reserved_sectors_num * sb->size_per_sector;
    data_offset =
        fat_offset + (sb->FATs_num * sb->sectors_per_FAT -
                      2 * sb->sectors_per_cluster) * /* 簇编号从2开始 */
                         sb->size_per_sector;
    return 0;
}

int disp_DBR_info() {
    fat_superblock_t *sb = &fat_superblock;
    printf("size_per_sector: %u (byte)\n", sb->size_per_sector);
    printf("sectors_per_cluster: %u\n", sb->sectors_per_cluster);
    printf("reserved_sectors_num: %u\n", sb->reserved_sectors_num);
    printf("FATs_num: %u\n", sb->FATs_num);
    printf("sectors_num: %u\n", sb->sectors_num);
    printf("sectors_per_FAT: %u\n", sb->sectors_per_FAT);
    printf("root_clus: %u\n", sb->root_clus);
    return 0;
}

int read_clus(u32 cluster, u8 *buf) {
    fat_superblock_t *sb = &fat_superblock;
    u32 cluster_size = sb->sectors_per_cluster * sb->size_per_sector;
    read_offset(img_fd, data_offset + cluster * cluster_size, cluster_size,
                buf);
    memcpy(img_buf, buf, cluster_size);
    return 0;
}

u32 next_clus(u32 cluster) {
    u32 nxt = 0xFFFFFFFFU; /* Next cluster */
    u8 *p = img_buf + fat_offset + cluster * 4;
    read_offset(img_fd, fat_offset + cluster * 4, 4, p);
    nxt = *(u32 *)(p);
    return nxt;
}

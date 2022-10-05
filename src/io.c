#include "file.h"
#include <malloc.h>
#include "io.h"

fat_superblock_t fat_superblock;
u8 buf[BUF_SIZE];

int img_fd;

int load_disk_image(char *imagepath) {
    return open_file(&img_fd, imagepath, O_RDWR);
}

int load_DBR_info() {
    static int err;
    fat_superblock_t* sb = &fat_superblock;
    err = read_offset(img_fd, 0, 0x200, buf);
    if(err) return err;
    sb->size_per_sector = *(u16*)(buf + 0x0B);
    sb->sectors_per_cluster = *(u8*)(buf + 0x0D);
    sb->reserved_sectors_num = *(u16*)(buf + 0x0E);
    sb->FATs_num = *(u8*)(buf + 0x10);
    sb->sectors_num = *(u32*)(buf + 0x20);
    sb->sectors_per_FAT = *(u32*)(buf + 0x24);
    sb->root_clus = *(u32*)(buf + 0x2C);
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
#include "file.c"
#include <malloc.h>

#define BUF_SIZE 1000000

u8 buf[BUF_SIZE];
int img_fd;
u16 size_per_sector;
u8 sectors_per_cluster;
u16 reserved_sectors_num;
u8 FATs_num;
u32 sectors_num;
u32 sectors_per_FAT;
u32 root_clus;

int load_disk_image(char *imagepath) {
    return open_file(&img_fd, imagepath, O_WRONLY);
}

int load_DBR_info() {
    static int err;
    err = read_offset(img_fd, 0, 0x200, buf);
    if(err) return err;
    size_per_sector = *(u16*)(buf + 0x0B);
    sectors_per_cluster = *(u8*)(buf + 0x0D);
    reserved_sectors_num = *(u16*)(buf + 0x0E);
    FATs_num = *(u8*)(buf + 0x10);
    sectors_num = *(u32*)(buf + 0x20);
    sectors_per_FAT = *(u32*)(buf + 0x24);
    root_clus = *(u32*)(buf + 0x2C);
    return 0;
}

int disp_DBR_info() {
    printf("size_per_sector: %u (byte)\n", size_per_sector);
    printf("sectors_per_cluster: %u\n", sectors_per_cluster);
    printf("reserved_sectors_num: %u\n", reserved_sectors_num);
    printf("FATs_num: %u\n", FATs_num);
    printf("sectors_num: %u\n", sectors_num);
    printf("sectors_per_FAT: %u\n", sectors_per_FAT);
    printf("root_clus: %u\n", root_clus);
    return 0;
}
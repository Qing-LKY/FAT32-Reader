#define __GNU_SOURCE
#include "fat.h"
#include "io.h"
#include <string.h>
#include <uchar.h>
#include <stdio.h>
#include <stdlib.h>

int fat_parse_entry(fat_entry_t *entry, u8 *buf) {
    entry->attr = *(u8 *)(buf + 0x0B);
    if (entry->attr == ENTRY_ATTR_LFN) {
        entry->i_first = 0;
        entry->size = 0;
        entry->name = (char16_t *)calloc(14, sizeof(char16_t));

        memcpy((char16_t *)entry->name, (char16_t *)(buf + 0x1), 10);
        memcpy((char16_t *)entry->name + 5, (char16_t *)(buf + 0xE), 12);
        memcpy((char16_t *)entry->name + 11, (char16_t *)(buf + 0x1C), 4);

        char16_t *p_name = entry->name;
        for (; *p_name != u'\0' && p_name <= (char16_t *)entry->name + 13; p_name++) {
            ;
        }
        if (p_name == (char16_t *)entry->name + 13) {
            ((char16_t *)entry->name)[13] = u'\0';
            entry->n_len = 13;
        } else {
            entry->n_len = p_name - (char16_t *)entry->name;
        }
    } else {
        entry->i_first = *(u16 *)(buf + 0x14) << 16 | *(u16 *)(buf + 0x1A);
        entry->name =
            (char *)calloc(13, sizeof(char)); /* 文件名(8)+.+扩展名(3)+'\0' */
        char *name_end = stpncpy(entry->name, (char *)buf, 8); /* 文件名 */
        char *ext_start = name_end == entry->name ? entry->name : name_end + 1;
        *(ext_start++) = '.';
        char *ext_end = stpncpy(ext_start, (char *)(buf + 0x8), 3); /* 扩展名 */
        if (ext_end == ext_start) {
            --ext_start;
            --ext_end; /* 空扩展名需要去掉添加的'.' */
            *ext_start = '\0';
        } else {
            *(ext_end + 1) = '\0';
        }
        entry->n_len = ext_end - (char *)entry->name;
        entry->size = *(u32 *)(buf + 0x1C);
    }
    return 0;
}

fat_entry_t *fat_parse_dir(fat_entry_t *e, int *len) {
    if (e->attr != 0x20) {
        perror("Not a directory");
        return NULL;
    }

    fat_superblock_t *sb = &fat_superblock;
    int cluster_size = sb->sectors_per_cluster * sb->size_per_sector;
    int entries_per_cluster = cluster_size / ENTRY_SIZE;

    u8 *buf = (u8 *)malloc(cluster_size);
    if (!buf) return NULL;
    fat_entry_t *entries =
        (fat_entry_t *)calloc(entries_per_cluster, sizeof(fat_entry_t));
    *len = entries_per_cluster;
    if (!entries) goto bufclean;
    u8 *entry_buf = NULL;
    fat_entry_t *entry = entries; /* 指向待填充的目录项 */
    int clus = e->i_first;
    while (1) {
        // read cluster
        if (read_clus(clus, buf) < 0) goto entriesclean;

        // parse entries in the cluster
        entry_buf = buf;
        for (int i = 0; i < entries_per_cluster; i++) {
            fat_parse_entry(entry, entry_buf);
            if (entry->n_len != 0) {
                // 去除空项
                ++entry;
            }
            entry_buf += ENTRY_SIZE;
        }
        *len = entry - entries;
        // TODO: Bad cluster
        clus = next_clus(clus);
        if (CLUS_IS_END(clus)) {
            goto bufclean;
        } else {
            fat_entry_t *new_entries =
                realloc(entries, *len + entries_per_cluster);
            if (!new_entries) goto entriesclean;
            entries = new_entries;
            entry = entries + *len;
        }
    }
entriesclean:
    /* Failed parsing */
    for (fat_entry_t *p = entries; p != entry; ++p)
        if (p->name) free(p->name);
    free(entries);
    entries = NULL;
    entry = NULL;
bufclean:
    free(buf);
    return entries;
}

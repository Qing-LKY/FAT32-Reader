#define __GNU_SOURCE
#include "fat.h"
#include "io.h"

int fat_parse_entry(fat_entry_t *entry, u8 *buf) {
    entry->attr = *(u8 *)(buf + 0x0B);
    if (entry->attr == ENTRY_ATTR_LFN) {
        entry->size = 0;
        entry->ordinal = buf[0];
        entry->name = (char *)calloc(14, sizeof(char));

        int i_chars[] = {1, 3, 5, 7, 9, 14, 16, 18, 20, 22, 24, 28, 30};
        int i = 0;
        for (; i < 13; i++) {
            u16 c = *(u16 *)(buf + i_chars[i]);
            if ((u8)c == '\0') break;
            entry->name[i] = c;
        }
        entry->name[i] = '\0';
        entry->n_len = i;
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
        // 长度有问题
        entry->n_len = ext_end - (char *)entry->name;
        entry->size = *(u32 *)(buf + 0x1C);
    }
    return 0;
}

// 将 head 至 tail 组成的长目录项，合并到 head 上
// 返回合并后，如果长目录项错误，则
int fat_merge_lfn(fat_entry_t *head, fat_entry_t *tail) {
    if (!(head->ordinal & 0b1000000))
        // 第7位为1
        return -1;

    u8 i_lfn = head->ordinal & ~(0b1000000);
    int n_len = 0;
    for (fat_entry_t *p = head;; p++) {
        n_len += p->n_len;
        if (p == tail) break;
    }
    char *name = malloc((n_len + 1) * sizeof(char)); /* +1 for '\0' */
    name[0] = '\0';

    char *p_name = name;
    for (fat_entry_t *p = head;; p++) {
        if ((p->ordinal & i_lfn) != i_lfn)
            // 长目录项序号未连续递减
            return -1;
        strncat(name, p->name, p->n_len);
        free(p->name);
        p_name += p->n_len;
        --i_lfn;
        if (p == tail) break;
    }
    *(++p_name) = u'\0';

    head->name = name;
    head->n_len = n_len;
    return 0;
}

fat_entry_t *fat_parse_dir(fat_entry_t *e, int *len) {
    if (e->attr != ENTRY_ATTR_DIR) {
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
    if (!entries) goto bufclean;
    u8 *entry_buf = NULL;

    int i_entry = 0;     // entries 中待填充的目录项
    int i_first_lfn = 0; // entries中未合并的长目录项的第一项的下标
                         // 使用下标而不是指针，防止出现entries重分配后地址变化

    int clus = e->i_first;
    while (1) {
        // read cluster
        if (read_clus(clus, buf) < 0) goto entriesclean;

        // parse entries in the cluster
        entry_buf = buf;
        for (int i = 0; i < entries_per_cluster; i++) {
            fat_parse_entry(&entries[i_entry], entry_buf);
            if (entries[i_entry].n_len != 0) {
                // 去除空项
                if (entries[i_entry].attr == ENTRY_ATTR_LFN) {
                    if (entries[i_entry].ordinal & 0b1000000)
                        i_first_lfn = i_entry;
                    if (entries[i_entry].ordinal == 1 ||
                        entries[i_entry].ordinal == 0b1000001) {
                        // 长目录项的最后一项
                        if (fat_merge_lfn(entries + i_first_lfn, entries + i)) {
                            i_entry = i_first_lfn - 1;
                        } else {
                            i_entry = i_first_lfn;
                        }
                    }
                } else {
                    // 对于短目录项，检查上一项是否为长目录项，是则合并
                    if (i_first_lfn == i_entry - 1 &&
                        (entries[i_first_lfn].ordinal & 0b1000000)) {
                        free(entries[i_entry].name);
                        entries[i_entry].name = entries[i_entry-1].name;
                        entries[i_entry-1] = entries[i_entry];
                        --i_entry;
                    }
                }
                ++i_entry;
            }
            entry_buf += ENTRY_SIZE;
        }
        *len = i_entry;
        // TODO: Bad cluster
        clus = next_clus(clus);
        if (CLUS_IS_END(clus)) {
            goto bufclean;
        } else {
            fat_entry_t *new_entries =
                realloc(entries, *len + entries_per_cluster);
            if (!new_entries) goto entriesclean;
            entries = new_entries;
        }
    }
entriesclean:
    /* Failed parsing */
    for (int i = 0; i < i_entry; i++)
        if (entries[i_entry].name) free(entries[i_entry].name);
    free(entries);
    entries = NULL;
bufclean:
    free(buf);
    return entries;
}

int fat_read_file(fat_entry_t *e, u8 *buf, int size) {
    if(e->size > size) return -1;
    fat_superblock_t *sb = &fat_superblock;
    int clus = e->i_first;
    int mx_clus = (int)sb->size_per_sector / 4 * (int)sb->sectors_per_FAT;
    int siz_clus = (int)sb->sectors_per_cluster * (int)sb->size_per_sector;
    u8 *now = buf;
    while(clus < mx_clus) {
        int err = read_clus(clus, now);
        if(err) return -1;
        now += siz_clus;
        clus = next_clus(clus);
    }
    return 0;
}

int fat_to_file(fat_entry_t *e, int target_fd) {
    if(e->attr != ENTRY_ATTR_ARC) return -1;
    fat_superblock_t *sb = &fat_superblock;
    int rest = e->size, clus = e->i_first;
    int mx_clus = (int)sb->size_per_sector / 4 * (int)sb->sectors_per_FAT;
    int siz_clus = (int)sb->sectors_per_cluster * (int)sb->size_per_sector;
    u8 *buf = (u8 *)malloc(siz_clus);
    int err = 0;
    while(clus < mx_clus) {
        int err = read_clus(clus, buf);
        if(err != 0) break;
        int sz = rest > siz_clus ? siz_clus : rest;
        write(target_fd, buf, sz);
        rest -= sz;
        clus = next_clus(clus);
    }
    free(buf);
    return err;
}

int free_entry_array(fat_entry_t *arr, int n) {
    for(int i = 0; i < n; i++) free(arr[i].name);
    free(arr);
    return 0;
}

void entry_copy(fat_entry_t *des, fat_entry_t *src) {
    des->attr = src->attr;
    des->i_first = src->i_first;
    des->n_len = src->n_len;
    des->name = (char *)malloc(src->n_len + 1);
    strcpy(des->name, src->name);
    des->size = src->size;
}
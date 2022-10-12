#include <check.h>
#include <stdlib.h>
#include <uchar.h>
#include "fat.h"
#include "file.h"
#include "io.h"

START_TEST(test_parse_dir) {
    char *s = "./a.img";
    load_disk_image(s);
    load_DBR_info();
    fat_entry_t root_entry = {
        .attr = 0x10,
        .i_first = 2,
        .n_len = 1,
        .name = "/",
        .size = 0
    };
    int len = 0;
    fat_entry_t *entries = fat_parse_dir(&root_entry, &len);
    for (int i = 0; i < len; i++) {
        printf("%s\n", entries[i].name);
    }
    ck_assert_msg(len == 4, "len == 4 failed, len=%d", len);
}
END_TEST

START_TEST(test_parse_entry_LFN) {
    unsigned char buf[32] = {
        0x41, 0x74, 0x00, 0x65,
        0x00, 0x78, 0x00, 0x74,
        0x00, 0x44, 0x00, 0x0f,
        0x00, 0x82, 0x69, 0x00,
        0x72, 0x00, 0x31, 0x00,
        0x00, 0x00, 0xff, 0xff,
        0xff, 0xff, 0x00, 0x00,
        0xff, 0xff, 0xff, 0xff
    };
    fat_entry_t entry;
    fat_parse_entry(&entry, buf);
    ck_assert_mem_eq(entry.name, u8"textDir1", entry.n_len + 1);
}
END_TEST

START_TEST(test_merge_lfn) {

}
END_TEST

Suite * suite_fat(void) {
    Suite *s;
    TCase *tc_parse_dir;
    s = suite_create("Fat");
    tc_parse_dir = tcase_create("fat_parse_dir");
    tcase_add_test(tc_parse_dir, test_parse_dir);
    suite_add_tcase(s, tc_parse_dir);

    TCase *tc_parse_entry_LFN = tcase_create("fat_parse_entry_LFN");
    tcase_add_test(tc_parse_entry_LFN, test_parse_entry_LFN);
    suite_add_tcase(s, tc_parse_entry_LFN);
    return s;
}

START_TEST(test_DBR_info) {
    // TODO: Use relative path instead.
    // Cannot find where a.img should be put though.
    char *s = "./a.img";
    int code;
    code = load_disk_image(s);
    ck_assert(img_fd > 0);
    ck_assert(code == 0);
    load_DBR_info();
    ck_assert(fat_superblock.root_clus == 2);
    ck_assert(fat_superblock.size_per_sector == 512);
    ck_assert(fat_superblock.sectors_per_cluster == 1);
    ck_assert(fat_superblock.reserved_sectors_num == 32);
    ck_assert(fat_superblock.FATs_num == 2);
    ck_assert(fat_superblock.sectors_num == 81920);
    ck_assert(fat_superblock.sectors_per_FAT == 630);
}
END_TEST

START_TEST(test_next_clus) {
    char *s = "./a.img";
    load_disk_image(s);
    load_DBR_info();
    u32 cluster = 4;
    u32 nxt = next_clus(cluster);
    ck_assert_msg(nxt == 0x00000005, "nxt: %d", nxt);
}
END_TEST

START_TEST(test_read_clus) {
    char *s = "./a.img";
    load_disk_image(s);
    load_DBR_info();
    u8 *buf = (u8 *)malloc(fat_superblock.sectors_per_cluster * fat_superblock.size_per_sector);
    read_clus(4, buf);
    ck_assert_msg(buf[0] == 0x50, "buf[0] == 0x50 failed, buf[0] = %u", buf[0]);
}
END_TEST

Suite * suite_io(void) {
    Suite *s;
    TCase *tc_DBR_info;
    s = suite_create("IO");
    tc_DBR_info = tcase_create("disp_DBR_info");
    tcase_add_test(tc_DBR_info, test_DBR_info);
    suite_add_tcase(s, tc_DBR_info);

    TCase *tc_next_clus;
    tc_next_clus = tcase_create("next_clus");
    tcase_add_test(tc_next_clus, test_next_clus);
    suite_add_tcase(s, tc_next_clus);
    
    TCase *tc_read_clus;
    tc_read_clus = tcase_create("read_clus");
    tcase_add_test(tc_read_clus, test_read_clus);
    suite_add_tcase(s, tc_read_clus);
    return s;
}

int main() {
    int number_failed = 0;
    SRunner *sr;

    sr = srunner_create(suite_fat());
    srunner_add_suite(sr, suite_io());

    srunner_run_all(sr, CK_VERBOSE);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
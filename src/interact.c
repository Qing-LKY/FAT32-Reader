#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "io.h"
#include "fat.h"

const char *cutline = "\n======================================\n";
const char *cmd_cd = "cd";
const char *cmd_ls = "ls";
const char *cmd_load = "load";
const char *cmd_dump = "dump";
const char *cmd_exit = "exit";


char *img_path;
char *file_path;
char *cmd;

fat_superblock_t *sb = &fat_superblock;
fat_entry_t root, now;

int loaded = 0;
char *cwd[BUF_SIZE]; int ccwd;

int reset_cwd() {
    for(int i = 0; i < ccwd; i++) free(cwd[i]);
    ccwd = 1;
    cwd[0] = (char *)malloc(2);
    cwd[0][0] = '/', cwd[0][1] = 0;
    return 0;
}

int roll_back_cwd() {
    ccwd--;
    free(cwd[ccwd]);
    return 0;
}

int next_cwd(char *name, int n) {
    if(n == 2 && name[0] == '.' && name[1] == '.') {
        roll_back_cwd();
    } else {
        cwd[ccwd] = (char *)malloc(n + 1);
        memcpy(cwd[ccwd], name, n + 1);
        ccwd++;
    }
    return 0;
}

int print_cwd() {
    printf("[");
    if(loaded) print_string(img_path);
    printf(":");
    for(int i = 1; i < ccwd; i++) {
        putchar('/');
        print_string(cwd[i]);
    }
    if(ccwd == 1) putchar('/');
    printf("]$ ");
    return 0;
}

int interact_load_image() {
    /* 读入磁盘映像的位置 */
    img_path = readline("Input the path of your image (~/a.img): ");
    add_history(img_path);
    /* 载入磁盘映像 */
    int err = load_disk_image(img_path);
    if(err != 0) {
        printf("You got an error (no.%d) when loading image!\n", err);
        return -1;
    }
    err = load_DBR_info();
    if(err != 0) {
        printf("You got an error (no.%d) when reading DBR!\n", err);
        return -1;
    }
    fat_entry_t tmp = {
        .name = NULL,
        .attr = ENTRY_ATTR_DIR,
        .i_first = sb->root_clus,
        .size = 0
    };
    root = now = tmp;
    reset_cwd(); /* 修改显示的 cwd */
    puts("Load successfully!");
    return 0;
}

int parse_filepath(char *s, int n) {
    int l = 1, r = 1;
    if(s[0] != '/') l = r = 0;
    else now = root;
    while(l < n) {
        /* 解析出当前目录下的目录项 */
        int len;
        fat_entry_t *arr = fat_parse_dir(&now, &len);
        /* s[l...r-1] 是目标目录的名字 */
        while(r < len && s[r] != '/') r++;
        /* 尝试寻找 */
        u8 attr = ENTRY_ATTR_DIR;
        s[r] = 0;
        int flag = 0;
        for(int i = 0; i < len; i++) {
            if(arr[i].attr == attr && strcmp(s + l, arr[i].name) == 0) {
                flag = 1;
                free(now.name);
                entry_copy(&now, arr + i); /* 找到后，将 now 移动过去 */
                if(now.i_first == 0) now.i_first = sb->root_clus;
                next_cwd(now.name, now.n_len); /* 更新 cwd */
                break;
            }
        }
        /* free array */
        free_entry_array(arr, len);
        if(!flag) {
            printf("Dir %s not found!\n", s + l);
            return -1;
        }
        l = r + 1, r = l;
    }
    return 0;
}

int interact_change_path() {
    /* 读入目标文件在磁盘中的目录 */
    file_path = readline("Input the target file path (/TestDir): ");
    add_history(file_path);
    /* 解析字符串 寻找目录项 */
    int err = parse_filepath(file_path, strlen(file_path));
    if(err == -1) {
        puts("Fail to reach target!");
        return -1;
    }
    puts("Change successfully!");
    return 0;
}

// 显示这个文件夹的信息
int show_files() {
    if(now.attr != ENTRY_ATTR_DIR) {
        puts("Not a directory! Maybe you are going to dump it?");
        return -1;
    }
    int len;
    fat_entry_t *arr = fat_parse_dir(&now, &len);
    for(int i = 0; i < len; i++) {
        if(arr[i].attr == ENTRY_ATTR_DIR) printf("Dir: ");
        else printf("File: ");
        print_string(arr[i].name);
        puts("");
    }
    return 0;
}

int interact_dump_file() {
    /* 读入目标文件名 */
    file_path = readline("Input the target file in this path (test.doc): ");
    add_history(file_path);
    /* 寻找目标文件 */
    int len, flag = 0;
    fat_entry_t *arr = fat_parse_dir(&now, &len);
    fat_entry_t tar;
    for(int i = 0; i < len; i++) {
        if(arr[i].attr == ENTRY_ATTR_DIR) continue;
        if(strcmp(arr[i].name, file_path) == 0) {
            flag = 1;
            entry_copy(&tar, arr + i); /* 保存找到的项 */
            break;
        }
    }
    free_entry_array(arr, len); /* 释放数组 */
    if(flag == 0) {
        puts("File not found!");
        return -1;
    }
    puts("We found the entry!");
    /* 读入备份文件名 */
    puts("");
    file_path = readline("Now tell me what file to write (~/me.txt): ");
    add_history(file_path);
    /* 打开(创建)备份文件 */
    int tar_fd = open(file_path, O_WRONLY | O_CREAT, S_IRWXU);
    if(tar_fd == -1) {
        puts("Fail when create or open file!");
        return -1;
    }
    /* 写入文件并关闭 */
    if(fat_to_file(&tar, tar_fd) == -1) {
        close(tar_fd);
        puts("Error when write file!");
        return -1;
    } else close(tar_fd);
    puts("Dump successfully!");
    return 0;
}

int interact_normal() {
    if (loaded == 0) {
        cmd = readline("(FAT) ");
    } else {
        char s[30];
        snprintf(s, 30, "[%s] ", cwd[ccwd - 1]);
        cmd = readline(s);
    }
    add_history(cmd);
    if(strcmp(cmd, cmd_cd) == 0) {
        if(loaded == 0) {
            puts("Please load first!");
            return -1;
        }
        return interact_change_path();
    } else if(strcmp(cmd, cmd_ls) == 0) {
        if(loaded == 0) {
            puts("Please load first!");
            return -1;
        }
        return show_files();
    } else if(strcmp(cmd, cmd_dump) == 0) {
        if(loaded == 0) {
            puts("Please load first!");
            return -1;
        }
        return interact_dump_file();
    } else if(strcmp(cmd, cmd_load) == 0) {
        if(loaded) {
            loaded = 0;
            close(img_fd);
        }
        if(interact_load_image() == 0) {
            loaded = 1;
            return 0;
        } else return -1;
    } else if(strcmp(cmd, cmd_exit) == 0) {
        if(loaded) close(img_fd);
        puts("See you next time!");
        exit(0);
    } else {
        puts("Unknown Command. Following commands is supported:");
        puts("load -- to start a image loading");
        puts("cd -- to start a directory changing");
        puts("ls -- to show the files in cwd");
        puts("dump -- to start a file dumping");
        puts("exit -- to exit");
        return -1;
    }
    return 0;
}
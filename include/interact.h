#ifndef _FAT_INTERACT_H
#define _FAT_INTERACT_H

#include "io.h"
#include "fat.h"

// 重置显示当前位置的字符串到根
int reset_cwd();

// 回到父目录
int roll_back_cwd();

// 更新 cwd
int next_cwd(char *name, int n);

// 输出当前目录，作为输入的前缀
int print_cwd();

// 引导用户载入磁盘映像 成功返回 0
int interact_load_image();

// 引导用户移动地址
int interact_change_path();

// 解析用户读入的文件目录
int parse_filepath(char *s, int n);

// 显示这个文件夹的信息
int show_files();

// 引导用户导出文件
int interact_dump_file();

// 引导用户输入指令
int interact_normal();

#endif //_FAT_INTERACT_H
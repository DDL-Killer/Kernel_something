#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include<getopt.h>
// 参数标志位
#define FLAG_a (1 << 0)
#define FLAG_l (1 << 1)
#define FLAG_R (1 << 2)
#define FLAG_t (1 << 3)
#define FLAG_r (1 << 4)
#define FLAG_i (1 << 5)
#define FLAG_s (1 << 6)

int g_flags = 0; // 全局变量存参数

// 文件信息结构体，存你需要的所有东西，方便排序
typedef struct {
    char name[256];
    char path[4096]; // 完整路径，用于 lstat
    struct stat st;
} FileInfo;

// 比较函数原型
int cmp_name(const void *a, const void *b);
int cmp_time(const void *a, const void *b);

// 核心函数
void do_ls(const char *path);
void print_file_info(FileInfo *f);
void mode_to_str(mode_t mode, char *str);

int main(int argc, char *argv[]) {
    // 1. 解析参数 (建议手动解析或用 getopt)
    int opt;
    while ((opt = getopt(argc, argv, "alRtris")) != -1) {
        switch (opt) {
            case 'a': g_flags |= FLAG_a; break;
            case 'l': g_flags |= FLAG_l; break;
            case 'R': g_flags |= FLAG_R; break;
            // ... 其他参数
        }
    }

    // 2. 处理输入路径
    if (optind == argc) {
        do_ls("."); // 默认当前目录
    } else {
        // 处理多个目录参数 ls /home /etc
        // 注意：如果是多目录，且有 -t，标准 ls 会先对这几个目录名排序吗？(通常是分开处理)
        // 但题目要求 ls -t 会对内容排序
        for (int i = optind; i < argc; i++) {
            printf("%s:\n", argv[i]); // 多目录时打印目录名头
            do_ls(argv[i]);
        }
    }
    return 0;
}

void do_ls(const char *dir_path) {
    DIR *dir = opendir(dir_path);
    if (dir == NULL) {
        return;
    }

    struct dirent *entry;
    FileInfo *files = NULL;
    int count = 0;

    // 第一遍：读取所有文件到内存
    while ((entry = readdir(dir)) != NULL) {
        if (!(g_flags & FLAG_a) && entry->d_name[0] == '.') continue;

        // 动态扩容数组 (realloc)
        files = realloc(files, (count + 1) * sizeof(FileInfo));
        
        strncpy(files[count].name, entry->d_name, 255);
        // 拼接完整路径用于 lstat
        snprintf(files[count].path, 4096, "%s/%s", dir_path, entry->d_name);
        
        if (lstat(files[count].path, &files[count].st) == -1) {
            // lstat 失败处理
        }
        count++;
    }
    closedir(dir);

    // 第二遍：排序
    if (g_flags & FLAG_t) {
        qsort(files, count, sizeof(FileInfo), cmp_time);
    } else {
        qsort(files, count, sizeof(FileInfo), cmp_name); // 默认字典序
    }
    
    // 如果有 -r，反转排序逻辑或反向遍历

    // 第三遍：打印
    // 如果是 -l，这里需要先循环一次计算最大列宽（为了对齐）
    long total_blocks = 0;
    for(int i=0; i<count; i++) total_blocks += files[i].st.st_blocks;
    if (g_flags & FLAG_l || g_flags & FLAG_s) printf("total %ld\n", total_blocks / 2); // Linux ls total 是 1k block

    for (int i = 0; i < count; i++) {
        print_file_info(&files[i]);
    }

    // 第四遍：递归 (-R)
    if (g_flags & FLAG_R) {
        for (int i = 0; i < count; i++) {
            if (S_ISDIR(files[i].st.st_mode)) {
                if (strcmp(files[i].name, ".") == 0 || strcmp(files[i].name, "..") == 0) continue;
                
                printf("\n%s:\n", files[i].path);
                do_ls(files[i].path); // 递归调用
            }
        }
    }

    // 释放内存
    free(files);
}

// // =================================================================
// //                    Function Implementations
// // =================================================================

// // 比较函数，用于 qsort 按名称排序
// int cmp_name(const void *a, const void *b) {
//     const FileInfo *fa = (const FileInfo *)a;
//     const FileInfo *fb = (const FileInfo *)b;
//     return strcmp(fa->name, fb->name);
// }

// // 比较函数，用于 qsort 按时间排序
// int cmp_time(const void *a, const void *b) {
//     const FileInfo *fa = (const FileInfo *)a;
//     const FileInfo *fb = (const FileInfo *)b;
//     if (fa->st.st_mtime < fb->st.st_mtime) return 1;
//     if (fa->st.st_mtime > fb->st.st_mtime) return -1;
//     return 0; // if times are equal, you might want a secondary sort
// }


// void mode_to_str(mode_t mode, char *str) {
//     strcpy(str, "----------");
//     if (S_ISDIR(mode)) str[0] = 'd';
//     if (S_ISLNK(mode)) str[0] = 'l';
//     if (S_ISCHR(mode)) str[0] = 'c';
//     if (S_ISBLK(mode)) str[0] = 'b';
//     if (mode & S_IRUSR) str[1] = 'r';
//     if (mode & S_IWUSR) str[2] = 'w';
//     if (mode & S_IXUSR) str[3] = 'x';
//     if (mode & S_IRGRP) str[4] = 'r';
//     if (mode & S_IWGRP) str[5] = 'w';
//     if (mode & S_IXGRP) str[6] = 'x';
//     if (mode & S_IROTH) str[7] = 'r';
//     if (mode & S_IWOTH) str[8] = 'w';
//     if (mode & S_IXOTH) str[9] = 'x';
// }

// void print_file_info(FileInfo *f) {
//     if (g_flags & FLAG_l) {
//         char modestr[11];
//         mode_to_str(f->st.st_mode, modestr);

//         struct passwd *pw = getpwuid(f->st.st_uid);
//         struct group  *gr = getgrgid(f->st.st_gid);
        
//         char time_buf[32];
//         strftime(time_buf, sizeof(time_buf), "%b %d %H:%M", localtime(&f->st.st_mtime));

//         printf("%s %2ld %s %s %8ld %s %s\n",
//                modestr,
//                (long)f->st.st_nlink,
//                pw ? pw->pw_name : "unknown",
//                gr ? gr->gr_name : "unknown",
//                (long)f->st.st_size,
//                time_buf,
//                f->name);
//     } else {
//         printf("%s  ", f->name);
//     }
// }
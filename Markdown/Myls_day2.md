# 开始写myls的第二天

## 两个重要的函数

### `stat()`

```c
#include<sys/types.h>
#include<sys/stat.h>
#inlude<unistd.h>

int stat(const char *pathname,struct stat *statbuf);
```

* pathname:文件的路径(字符串)
* statbuf:指向`struct stat`结构体的指针,函数会将获取到的信息填入这个结构体中
* 返回值:成功返回0,失败返回-1并设置`errno`

* `stat()`会穿透,如果`pathname`是一个符号链接,它会跟踪(follow)这个链接,返回的是指向目标文件的信息
* 用途:想要读取文件的实际内容或属性时使用

## `lstat()`

```c
#include<sys/types.h>
#include<sys/stat.h>
#inlude<unistd.h>

int lstat(const char *pathname,struct stat *statbuf);
```

* 如果 `pathname` 是一个符号链接，`lstat()` **不会追踪**链接，它返回的是**符号链接文件本身**的信息。

* *用途：* 当你需要查看链接本身的信息（例如链接本身占用的空间、链接的权限）或者在编写像 `ls -l`、`tar` 备份工具时使用（防止陷入循环链接或备份了重复数据）

## `struct stat`结构体

```c
struct stat {
    dev_t     st_dev;     // 包含该文件的设备 ID
    ino_t     st_ino;     // Inode 节点号 (文件的唯一标识)
    mode_t    st_mode;    // 文件的类型和权限 (非常重要!)
    nlink_t   st_nlink;   // 硬链接数量
    uid_t     st_uid;     // 所有者的用户 ID
    gid_t     st_gid;     // 所有者的组 ID
    dev_t     st_rdev;    // 设备 ID (如果是特殊设备文件)
    off_t     st_size;    // 文件大小 (字节)
    blksize_t st_blksize; // 文件系统 I/O 的块大小
    blkcnt_t  st_blocks;  // 分配的 512B 块数量
    
    // 时间戳 (在较新的 Linux 内核中精度为纳秒)
    struct timespec st_atim;  // 最后访问时间 (Access Time)
    struct timespec st_mtim;  // 最后修改时间 (Modification Time)
    struct timespec st_ctim;  // 状态改变时间 (Change Time)
};
```

无论调用哪个函数,成功后都会填充此结构体
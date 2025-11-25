#include<sys/stat.h>

//这四个函数，若成功，返回0；若出错，返回-1
int stat(const char *restrict pathname,struct stat *restrict buf);
//一旦给出pathname,stat函数将返回与此命名文件有关的信息结构
int fstat(int fd,struct stat *buf);
//fstat函数获得已在描述符fd上打开文件的有关信息
int lstat(const char *restrict pathname,struct stat *restrict buf);
//lstat函数类似于stat,但是当命名的文件是一个符号链接时，lstat返回该符号链接的有关信息，而不是由该符号链接所引用的文件的信息
int fstatat(int fd,const char *restrict pathname,struct stat* restrict buf,int flag);
//fstatat函数为一个相对于当前打开目录（由fd指向）的路径名返回文件统计信息。flag参数控制着是否跟随一个符号链接
/*
flag参数控制着是否跟随着一个符号链接
当AT_SYMLINK_NOFOLLOW 标志被设置时，fstatat不会跟随符号链接，而是返回符号链接本身的信息，否则在默认情况下，返回是符号链接所指向实际文件的信息
如果flag 的参数是 AT_FDCWD 并且pathname参数是一个相对路径名，fstatat会计算于当前目录的pathname参数，如果pathname是一个绝对路径，fd参数就会被忽略

第二个参数buf是一个指针，指向一个我们必须提供的结构，函数来填充由buf指向的结构
基本形式：
struct stat{
    __mode_t st_mode;
    __ino_t st_ino;
    __dev_t st_dev;
    __dev_t st_rdev;
    __nlink_t st_link;
    __uid_t st_uid;
    __gid_t st_gid;
    __off_t st_size;
    struct timespec st_atime;
    struct timespec st_mtime;
    struct timespec st_ctime;
    __blksize_t st_blksize;
    __blkcnt_t st_blocks;
};

其中timespec结构按照秒和纳秒定义了时间，至少包括下面两个字段
    __time_t tv_sec;
    long tv_nsec;
*/

/*
文件按类型包括如下几种
1. 普通文件：对普通文件的解释由处理该文件的应用程序进行
            一个值得注意的例外是二进制可执行文件，为了执行程序，内核必须理解其形式，所有二进制可执行文件都遵循一个标准化的格式，这种格式能使内核能够确定程序文本和数据的加载位置
2. 目录文件：这种文件包含了其他文件的名字以及指向与这些文件有关信息的指针。
            对一个目录文件具有读权限的任一进程都可以读该目录的内容，但只有内核可以直接写目录文件
3. 块特殊文件：这种类型的文件对设备带缓冲的访问，每次访问以固定长度为单位进行
4. 字符特殊文件：这种类型的文件提供对设备不带缓冲的访问，每次访问长度可变
5. FIFO：这种类型的文件用于进程间通信，有时也命名为管道
6. 嵌套字（socket）：这种类型的文件用于进程间的网络通信，也可用于在一台宿主机上进程之间的非网络通信
7. 符号链接（symbolic link）：这种类型的文件指向另一个文件
*/

/*
文件类型包含在stat结构中的st_mode成员中，用宏确定类型
S_ISREG()   普通文件
S_ISDIR()   目录文件
S_ISCHR()   字符特殊文件
S_ISBLK()   块特殊文件
S_ISFIFO()  管道或FIFO
S_ISLNK()   符号链接
S_ISSOCK()  套接字
*/

/*
POSIX.1允许时间将进程间通信（IPC）对象说明为文件，它们的参数是指向stat结构的指针
S_TYPEISMQ()    消息队列
S_TYPEISSEM()   信号量
S_TYPEISSHM()   共享存储对象
*/
#include<fcntl.h>
#include<stdio.h>
#include<unistd.h>


//调用open或openat函数可以打开和创建一个文件
#include<fcntl.h>
int open(const char *path,int oflag,.../*mode_t mode*/);
int openat(int fd,const char * path,int oflag,.../*mode_t mode*/);
//path是打开或者要创建的文件的名字，oflag参数可以用来说明此函数的多个选项，用以下常量或多个常量进行运算构成oflag参数
//O_RDONLY,O_WRONLY,O_RDWR,O_EXEC(只执行打开),O_SEARCH(只搜索打开)：必须指定一个且只能指定一个

//调用creat()创建一个文件
#include<fcntl.h>
int creat(const char *path,mode_t mode);
//此函数等效于 open(path,O_WRONLY | O_CREAT | O_TRUNC,mode);
//函数不足之处在于它以只写方式打开所创建的文件
//可以直接使用 open(path,O_RDWR | O_CREAT | O_TRUNC,mode);

//调用close函数关闭一个打开文件

#include<unistd.h>
int close(int fd);
//当一个进程终止时，内核自动关闭它所有的打开文件

//每一个打开文件都有一个与其相关联的“当前文件偏移量”。通常读、写、操作都从当前文件偏移量开始
#include<unistd.h>
off_t lseek(int fd,off_t offset,int whence);
//当whence是SEEK_SET,则将该文件的偏移量设置为距离文件开始处offset个字节
//当........SEEK_CUR,........................当前值加offset
//当........SEEK_END,........................文件长度加offset

//调用read函数从文件中读数据

#include<unistd.h>
size_t read(int fd,void *buy,size_t nbytes);
//成功返回独到的字节数
//有多种情况下实际读到的字节数小于要求读到的字节数：
/*
1. 读普通文件时，在读到要求的字节数之前已经到达了文件尾端
2. 从终端设备读取时，通常一次最多读取一行
3. 从网络读取时，网络中的缓冲机制可能造成返回值小于所要求的字节数
4. 当从某些面向记录的设备读时，一次最多返回一个记录
5. 当一个信号中断，而已经读取了部分数据
*/

//调用write函数向打开文件写数据

#include<unistd.h>
size_t write(int fd,const void *buf,size_t nbytes);
//若成功，返回已经写的字节数

/*
内核使用三种数据结构表示打开文件
（1）每个进程在进程表中都有一个记录项，记录项中包含一个打开文件描述符表，可以视为一个矢量，每一个描述符占用一项，与每一个文件描述符相关联的是：
1. 文件描述符标志（close_）
2. 指向一个文件表项的指针
（2）内核为所有打开文件维持一张文件表，每一个文件表项包含
1. 文件状态标志（读、写、添加、同步和非阻塞）
2. 当前文件偏移量
3. 指向该文件v节点表项的指针
（3）每个打开文件都有一个v节点结构（v_node）。v节点结构包含了文件类型和对此文件进行各种操作函数的指针。对于大部分文件，v节点还包含了该文件的i节点

任何要求多于一个函数调用的操作都不是原子操作，因为在两个函数调用之间，内核有可能会临时挂起进程
UNIX系统为这样的操作提供了一种原子操作方法，即在打开文件时设置O_APPEND标志，这样做使得内核在每次写操作前，都将进程的当前偏移量设置到该文件的尾端处
*/

//Single UNIX Specificati包含了这种XSI拓展，该拓展允许原子性地定位并执行I/O
#include<unistd.h>
size_t pread(int fd,void *buf,size_t nbytes,off_t offset);
size_t pwrite(int fd,const void * buf,size_t nbytes,off_t offset);
/*
1. 调用pread时，无法中断其定位和读操作
2. 不更新当前文件偏移量
*/

//下面两个函数都可以用来复制一个现有的文件描述符
#include<unistd.h>
int dup(int fd);
int dup2(int fd,int fd2);

/*
由dup返回的新文件描述符一定是当前可用文件描述符中的最小数值
对于dup2,可以用fd2参数指定新描述符的值
如果fd2已经打开，那么先关闭fd2
如果fd等于fd2,则返回fd2
*/

/*
（1）dup2是一个原子操作，而close和fcntl包括两个函数调用
（2）dup2和fcntl有一些不同的error
*/

//传统的UNIX系统在实现内核中设有缓冲区高速缓存或页高速缓存，大部分磁盘I/O都通过缓冲区进行
//当我们向文件写入数据时，内核通过先将数据复制到缓冲区内，然后排入队列，晚些时候再写入磁盘，被称为延迟写
//通常，当内核需要重用缓冲去来存放其他磁盘块数据时，会把所有延迟写数据块写入磁盘
//为了保证磁盘上实际文件系统与缓冲区中内容的一致性，UNIX提供了sync,fsync,fdatasync三个函数
#include<unistd.h>
int fsync(int fd);
int fdatasync(int fd);
void sync(int fd);

//sync知识将所有修改过的块缓冲区排入写队列，然后就返回，它并不等实际写磁盘操作结束
//通常，称为updata的系统守护进程周期性地调用sync函数，这就保证了定期冲洗内核的块缓冲区，命令sync(1)也相当于调用sync函数
//fsync函数只对由文件描述符fd指定的一个文件起作用，并且等待写磁盘操作结束才返回。fsync可用于数据库这样的应用程序，这种应用程序需要确保修改过的块立刻写到磁盘上
//fdatasync类似于fsync,但它只影响文件的数据部分，而fsync还会同步更新文件的属性

//fcntl函数可以改变已经打开的文件的属性
#include<fcntl.h>
int fcntl(int fd,int cmd,...);

/*
fcntl函数有下面5个功能
（1）复制一个已经有的描述符（ cmd = F_DUPFD（复制文件描述符，新文件描述符作为函数值返回） 或 F_DUPFD_CLOEXEC（复制文件描述符，设置与新文件描述符关联的FD_CLOEXEC文件描述符标志的值，返回新文件描述符））
（2）获取/设置文件描述符标志（cmd = F_GETFD（对应于fd的文件描述符标志作为函数值返回） 或 F_SETFD（对于fd设置文件描述符标志））
（3）获取/设置文件状态标志（cmd = F_GETFL（对应于fd的文件描述标志作为函数值返回） 或 F_SETFL（将文件状态标志设置为第三个参数的值，可以更改的标志为：O_APPEND O_NONBLOCK O_SYNC O_DSYNC O_RSYNC O_FDYNC O_ASYNC））
（4）获取/设置异步I/O所有权（cmd = F_GETOWN（获取当前接收SIGIO和SIGURG信号的进程ID或进程组ID） 或 F_SETOWN（设置接受SIGIO和SIGURG信号的进程ID或进程组ID））
（5）获取/设置记录锁（cmd = F_GETLK 、F_SETLK 或 F_SETLKW）

fcntl的返回值与命令有关，如果出错，所有命令都返回-1，如果成功则返回某个其他值
F_DUPFD 返回新的文件描述符
F_GETFD 返回相应的标志
F_GETFL 返回相应的标志
F_GETOWN 返回正的进程ID或负的进程组ID
*/

//ioctl函数一直是I/O操作的杂物箱，终端I/O是使用ioctl最多的地方
#include<unistd.h>
#include<sys/ioctl.h>
int ioctl(int fd,int request,...);

//较新的系统都提供名为/dev/fd的目录，其目录项是名为0、1、2等的文件，打开文件/dev/fd/n等效于复制描述符n
// /dev/fd把文件描述符映射指向底层物理文件的符号链接
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

int stat(const char *pathname,struct stat *statbuf);
```
# myshell

## getcwd()

获取当前工作目录

```c
#include<unistd.h>
char *getcwd(char *buf,size_t size);
```

* 功能:获取当前进程的工作目录的绝对路径
* 返回绝对路径字符串
* 需要自己提供缓冲区
* 失败返回NULL

## fflush()

将缓冲区的数据立即写入实际文件

```c
#include<stdio.h>
int fflush(FILE *stream);
```

* 程序退出后会自动刷新

## strcspn()

```c
#include<string.h>
size_t strcspn(const char *s,const char *reject);
```

返回字符串s开头连续不包含reject中任意字符的字符数

* 分割字符串
* 去除尾部换行符
* 验证字符串格式

## strtok()

```c
#include<string.h>
char *strtok(char *str,const char *delim);
```

将字符串分割成一系列由分隔符分割的标记

* 多种分隔符
* 解析CSV文件
* 处理连续分隔符

* **不能用于常量字符串,一般作用于字符数组**

## fork()

创建一个新的进程(子进程),该进程是调用进程(父进程)的副本

```c
#include<unistd.h>
pid_t fork(void);
```

* 返回值

| 返回值 | 含义                              |
| ------ | --------------------------------- |
| -1     | 创建失败(进程数达到上限,内存不足) |
| 0      | 在子进程中返回                    |
| >0     | 在父进程中返回,值为子进程的PID    |

* 复制进程
* 文件描述符共享

## execvp()

```c
#include<unistd.h>
int execvp(const char *file,char *const argv[]);
//file:程序名(自动搜索PATH环境变量)
//argv:参数数组,最后一个元素必须是NULL
```

在当前进程中加载并执行一个新程序(进程替换)

## waitpid()

```c
#include<sys/wait.h>
pid_t waitpid(pid_t pid,int *status,int options);

/*
1. pid :指定等待的进程
	>0	等待特定PID的子进程
	-1	等待任意子进程
	0 	等待同一进程组的任意子进程
	< -1 等待进程组ID等于PID的任意子进程 
	
2. status:退出状态指针

3. options:选项标志
	0	阻塞直到子进程终止
	WNOHANG		立即返回(无子进程终止返回0)
	WUNTRACED	也返回已停止的子进程
	WCONTINUED	也返回已恢复到子进程	
*/
```

等待指定子进程状态改变(终止 停止 恢复)

## getenv()

获取指定环境变量的值

```c
#include<stdlib.h>
char *getenv(const char *name);
```

* 返回值
  * 成功:返回指向环境变量值的指针(不能修改)
  * 失败:返回NULL

1. 根据环境变量改变程序行为
2. 安全的复制环境变量值
3. 检查敏感环境变量
4. 遍历所有环境变量

## chdir()

```c
#include<unistd.h>
int chdir(const char *path);
```

改变当前进程的工作目录

## setenv()

```c
#include<stdlib.h>
int setenv(const char *name,const char *value,int overwrite);
```

添加 修改或删除环境变量

| 参数      | 说明                                  |
| --------- | ------------------------------------- |
| name      | 环境变量名                            |
| value     | 环境变量值                            |
| overwrite | 是否覆盖已存在的变量(非0覆盖,0不覆盖) |

## signal()

设置指定信号的处理函数

```c
#include<signal.h>
void (*signal(int signum,void (*handler)(int)))(int);

//通常使用typedef 简化
typedef void (*sighandler_t)(int);
sighandler_t signal(int signum,sighandler_t handler);

//参数
signum:要处理的信号编号
handler:信号处理函数指针
```

| 信号    | 值       | 默认动作  | 说明            |
| ------- | -------- | --------- | --------------- |
| SIGINT  | 2        | 终止      | Ctrl+C          |
| SIGQUIT | 3        | 终止+core | Ctrl+\          |
| SIGKILL | 9        | 终止      | 不能被捕获\忽略 |
| SIGSTOP | 17,19,23 | 终止      | 不能被捕获\忽略 |
| SIGTERM | 15       | 终止      | 正常终止信号    |
| SIGSEGV | 11       | 终止+core | 段错误          |

---

### **标准信号列表 (Linux)**

| 编号  | 宏名称                | 默认动作 | 说明                         |
| ----- | --------------------- | -------- | ---------------------------- |
| 1     | `SIGHUP`              | Term     | 终端挂起或控制进程终止       |
| 2     | `SIGINT`              | Term     | 键盘中断 (Ctrl+C)            |
| 3     | `SIGQUIT`             | Core     | 键盘退出 (Ctrl+)             |
| 4     | `SIGILL`              | Core     | 非法指令                     |
| 5     | `SIGTRAP`             | Core     | 断点或陷阱指令               |
| 6     | `SIGABRT`             | Core     | abort() 函数调用             |
| 6     | `SIGIOT`              | Core     | IOT 陷阱 (同 SIGABRT)        |
| 7     | `SIGBUS`              | Core     | 总线错误                     |
| 8     | `SIGFPE`              | Core     | 浮点异常                     |
| 9     | `SIGKILL`             | Term     | **强制终止 (不可捕获/忽略)** |
| 10    | `SIGUSR1`             | Term     | 用户自定义信号1              |
| 11    | `SIGSEGV`             | Core     | 段错误 (无效内存访问)        |
| 12    | `SIGUSR2`             | Term     | 用户自定义信号2              |
| 13    | `SIGPIPE`             | Term     | 向无读端的管道写数据         |
| 14    | `SIGALRM`             | Term     | alarm() 定时器超时           |
| 15    | `SIGTERM`             | Term     | 软件终止信号 (默认 kill)     |
| 16    | `SIGSTKFLT`           | Term     | 栈错误 (Linux 专用)          |
| 17    | `SIGCHLD`             | Ign      | 子进程状态改变 (可忽略)      |
| 18    | `SIGCONT`             | Cont     | 继续暂停的进程               |
| 19    | `SIGSTOP`             | Stop     | **暂停进程 (不可捕获/忽略)** |
| 20    | `SIGTSTP`             | Stop     | 键盘停止 (Ctrl+Z)            |
| 21    | `SIGTTIN`             | Stop     | 后台进程读终端               |
| 22    | `SIGTTOU`             | Stop     | 后台进程写终端               |
| 23    | `SIGURG`              | Ign      | socket 紧急数据              |
| 24    | `SIGXCPU`             | Core     | CPU 时间超限                 |
| 25    | `SIGXFSZ`             | Core     | 文件大小超限                 |
| 26    | `SIGVTALRM`           | Term     | 虚拟定时器超时               |
| 27    | `SIGPROF`             | Term     | 分析定时器超时               |
| 28    | `SIGWINCH`            | Ign      | 窗口大小改变                 |
| 29    | `SIGIO`               | Term     | I/O 就绪 (或 SIGPOLL)        |
| 30    | `SIGPWR`              | Term     | 电源故障                     |
| 31    | `SIGSYS`              | Core     | 无效系统调用                 |
| 34-64 | `SIGRTMIN`~`SIGRTMAX` | Cont     | 实时信号 (可排队的)          |

#### **默认动作说明**

- **Term**：终止进程
- **Core**：终止进程并产生核心转储
- **Ign**：忽略信号
- **Stop**：暂停进程
- **Cont**：继续进程

### **三种 handler 参数取值**

| 特殊值        | 含义               | 说明                         |
| ------------- | ------------------ | ---------------------------- |
| **函数指针**  | 自定义处理函数     | 指向用户定义的信号处理函数   |
| **`SIG_DFL`** | Default (默认处理) | 执行信号的默认动作           |
| **`SIG_IGN`** | Ignore (忽略信号)  | 忽略该信号（不执行任何操作） |
| **`SIG_ERR`** | Error (错误返回值) | signal() 调用失败时的返回值  |

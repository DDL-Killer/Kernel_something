# 开始写myls的第一天

计划三天完成,今天在gemini的辅助下完成了第一天的进度:通过调用`getopt()`函数来判断命令行参数,通过`opendir()`函数获得`DIR`参数指针并且通过`readdir()`反复调用`dirent`信息打印`d_name`

## `getopt()`函数

* `getopt()`函数是标准库`<unistd.h>`提供的专门用来标准化解析命令行参数的函数

  ```c
  #include <unistd.h>
  
  int getopt(int argc, char * const argv[], const char *optstring);
  /*
  关键参数:optstring
  单个参数:表示这是一个开关,不需要参数 &quot;a&quot;表示接受-a
  字符后跟一个冒号:表示选项后面必须跟一个参数值
  字符后跟两个冒号:表示参数是可选的
  */
  ```

| 变量名 | 类型     | 作用                                      |
| ------ | -------- | ----------------------------------------- |
| optarg | `char *` | 指向当前选项的*参数值*                    |
| optind | `int`    | 下一个将被处理的argv元素的下标            |
| optopt | `int`    | 当发现未知选项时,存储那个位置的字符       |
| opterr | `int`    | 设为0可禁止getopt向stderr输出默认错误信息 |

## 位运算的代码实现思路

一个`int`存储32位,使用位运算,可以简化操作

## `dirent`库

## `-a`实现思路


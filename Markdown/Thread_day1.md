# 目标

理解`pthread`库的基本用法,理解并发核心思想:任务分解和结果合并,实现多线程排序

## pthread

* pthread是POSIX线程库,用于在C/C++中编写多线程程序

### pthread函数

| 函数             | 作用           | 必须掌握 |
| ---------------- | -------------- | -------- |
| `pthread_create` | 创建新线程     | ✅        |
| `pthread_join`   | 等待线程结束   | ✅        |
| `pthread_exit`   | 结束当前线程   | ✅        |
| `pthread_self`   | 获取当前线程ID | 常用     |
| `pthread_detach` | 分离线程       | 常用     |
| `pthread_cancel` | 取消线程       | 进阶     |

### pthread_create()  创建线程

```c
#include <pthread.h>

int pthread_create(pthread_t *thread,const pthread_attr_t *attr,void *(*start_routine)(void*),void *arg);
```

#### 参数详解

| 参数            | 类型                    | 含义                                         |
| --------------- | ----------------------- | -------------------------------------------- |
| `thread`        | `pthread_t*`            | 输出参数，线程创建成功后，线程ID存这里       |
| `attr`          | `const pthread_attr_t*` | 线程属性（优先级、栈大小等），传NULL用默认值 |
| `start_routine` | `void* (*)(void*)`      | 线程要执行的函数（函数指针）                 |
| `arg`           | `void*`                 | 传给线程函数的参数                           |

#### 返回值

* 0:成功
* 非0:失败,返回错误码(不是-1)

### 线程函数的固定格式

```c
void* my_thread_func(void* arg) {
    // 线程要做的事情
    return NULL;  // 或者 return 某个指针
}
```

### pthread_join()  等待线程

```c
int pthread_join(pthread_t thread, void **retval);
```

* 作用:
  1. 阻塞调用线程,直到目标线程结束
  2. 回收线程资源
  3. 获取线程的返回值

#### 参数详解

| 参数     | 含义                         |
| -------- | ---------------------------- |
| `thread` | 要等待的线程ID               |
| `retval` | 接收线程返回值，不想要传NULL |

#### 返回值

- **0**：成功
- **ESRCH**：找不到这个线程
- **EINVAL**：线程不可join（已经被分离过）
- **EDEADLK**：死锁（自己等自己）

### pthread_exit()  结束线程

```c
void pthread_exit(void *retval);
```

* 作用
  1. 主动结束当前线程
  2. 返回值通过`pthread_join`被其他线程获取

#### 与return的区别

| 方式                  | 效果                             |
| --------------------- | -------------------------------- |
| `return NULL`         | 线程正常结束，返回NULL           |
| `pthread_exit(NULL)`  | 线程结束，返回NULL               |
| 主线程 `return`       | 整个进程结束，所有线程被强制终止 |
| 主线程 `pthread_exit` | 主线程结束，其他线程继续运行     |

### pthread_self()  获取线程ID

```c
pthread_t pthread_self(void);
```

### pthread_detach()  分离线程

```c
int pthread_detach(pthread_t thread);
```

* 作用
  1. 线程结束后自动回收资源,不需要`pthread_join`
  2. 分离后的线程不能再被join

#### 对比总结

### 线程类型 vs 回收方式

| 类型             | 创建方式              | 回收方式            | 适用场景             |
| ---------------- | --------------------- | ------------------- | -------------------- |
| **可 join 线程** | 默认                  | `pthread_join` 等待 | 需要获取返回值       |
| **分离线程**     | 调用 `pthread_detach` | 自动回收            | 后台任务，不需要等待 |

### 编译时末尾要加 `-lpthread`

## 两路归并算法

```c
void merge(int left[],int left_len,int right[],int right_len,int result[]){
	int i = 0;//left
	int j = 0;//right
	int k = 0;//result
	
	while(i<left_len && j<right_len){
		if(left[i] <= right[j]){
			result[k++] = left[i++];
		}else{
			result[k++] = right[j++];
		}
	}
	
	while(i<left_len){
		result[k++] = left[i++];
	}
	while(j<right_len){
		result[k++] = right[j++];
	}
} 
```

## srand()

```c
#include <stdlib.h>

void srand(unsigned int seed);
```

用于设置随机数种子的函数,配合`rand()`

### time(NULL)

```c
time_t time(time_t *timer);
```

* 返回从 **1970-01-01 00:00:00 UTC** 到现在的**秒数**

* 参数传 `NULL` 表示不需要存储到变量

* 返回值类型 `time_t` 本质是整数

## rand()

生成伪随机整数的函数

```c
#include <stdlib.h>

int rand(void);
```

#### 生成[0,N-1]范围的随机数

```c
int r = rand() % N;
```

#### 生成[a,b]范围的随机数

```c
int r = rand() % (b - a + 1) + a;
```

#### 生成[0,1)的浮点数

```c
double r = (double)rand() / RAND_MAX;
```

#### 生成[a,b]的浮点数

```c
double r = a + (double)rand() / RAND_MAX * (b - a);
```

#### 局限性

1. 低位随机性差

   推荐:

   ```c
   // ❌ 不建议：低位的随机性不好
   int r = rand() % 2;  // 0和1可能不均匀
   
   // ✅ 更好：用高位
   int r = (rand() >> 4) % 2;
   ```

   

2. 周期有限:2^31

3. 非线程安全

4. 不适合安全场景

#### 速查表

| 需求       | 代码                        |
| ---------- | --------------------------- |
| 包含头文件 | `#include <stdlib.h>`       |
| 基本用法   | `int r = rand();`           |
| 0 到 N-1   | `rand() % N`                |
| a 到 b     | `rand() % (b-a+1) + a`      |
| 0.0 到 1.0 | `(double)rand() / RAND_MAX` |
| 初始化种子 | `srand(time(NULL))`         |
| 最大值的宏 | `RAND_MAX`                  |

## struct timeval

`struct timeval`是linux/Unix系统中用于表示精确时间的结构体,可以精确到微妙

```c
#include <sys/time.h>

struct timeval {
    time_t tv_sec;     // 秒数 (seconds)
    suseconds_t tv_usec; // 微秒数 (microseconds)
};
```

| 成员      | 类型          | 范围        | 说明                          |
| --------- | ------------- | ----------- | ----------------------------- |
| `tv_sec`  | `time_t`      | 0 到很大    | 从1970-01-01开始的秒数        |
| `tv_usec` | `suseconds_t` | 0 到 999999 | 微秒数（1秒 = 1,000,000微秒） |

## gettimeofday()

```c
#include <sys/time.h>

int gettimeofday(struct timeval *tv, struct timezone *tz);
```

| 参数 | 说明                                    |
| ---- | --------------------------------------- |
| `tv` | 输出参数，获取的时间存这里              |
| `tz` | 时区信息，**现在总是传 NULL**（已废弃） |

## 堆排序

一种基于二叉堆数据结构的排序算法,时间复杂度O(n log n),空间复杂度O(1)

#### 类型

| 类型       | 特点            | 用途     |
| ---------- | --------------- | -------- |
| **大顶堆** | 父节点 ≥ 子节点 | 升序排序 |
| **小顶堆** | 父节点 ≤ 子节点 | 降序排序 |

#### 算法

1. 建堆:将无序数组调整为最大堆
2. 排序:每次将堆顶元素与末尾元素交换,将堆的最大规模减一,调整剩余元素为最大堆,重复该过程直到只剩一个元素

#### 实现(大项)

```c
#include<stdio.h>

//交换两个数
void swap(int *a,int *b){
    int temp = *a;
    *a = *b;
    *b = temp;
}

//堆调整
//arr:数组 n:堆大小 i:要调整的节点索引
void heapify(int arr[],int n,int i){
    int largest = 1;//假设当前节点最大
    int left = 2 * i + 1;
    int right = 2 * i + 2;
    
    //如果左子节点存在且大于当前最大
    if(left < n && arr[left] > arr[largest]){
        largest = left;
    }
    
    //如果右子节点存在且大于当前最大
    if(right < n && arr[right] > arr[largest]){
        largest = right;
    }
    
    //如果最大不是当前节点
    if(largest != i){
        swap(&arr[i],&arr[largest]);
        heapify(arr,n,largest); //递归调整交换的子节点
    }
}

//堆排序主函数
void heapSort(int arr[],int n){
    //构建大顶堆(从最后一个非叶节点开始)
    //最后一个非叶节点索引 = n/2 -1
    for(int i = n/2 -1;i>=0;i--){
        heapify(arr,n,i);
    }
    
    //取出堆顶元素
    for(int i = n - 1;i>0;i--){
        //堆顶换到最后
        swap(&arr[0],&arr[i]);
        //调整剩余堆
        heapify(arr,i,0);
    }
}
```


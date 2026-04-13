#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>
#include<sys/time.h>
#include<semaphore.h>
#include<unistd.h>

//任务节点(单向链表)
typedef struct task{
    void (*function)(void* arg);  //函数指针,指向要执行的任务
    void* arg;  //任务函数的参数
    struct task* next;  //指向下一个任务
} task_t;

//线程池结构体
typedef struct{
    pthread_mutex_t lock;  //互斥锁:保护任务队列
    pthread_cond_t notify;  //条件变量:队列有任务时唤醒工作线程
    pthread_t *threads;  //工作线程组
    task_t *queue_head;  //队列头
    task_t *queue_tail;  //队列尾
    int thread_count;  //线程数量
    int task_count;  //当前等待的任务数
    int shutdown;  //关门销毁标志
} threadpool_t;

//工作函数
void* worker_thread(void *threadpool){
    //实例传入参数,获取控制器信息
    threadpool_t *control = (threadpool_t* )threadpool;

    while(1){
        //获取队列锁
        pthread_mutex_lock(&(control->lock));

        //如果队列为空,且没有关门指令,休眠等待指令
        while((control->task_count == 0) && (!control->shutdown)){
            pthread_cond_wait(&(control->notify),&(control->lock));
        }

        //收到关门且队列无积压任务,准备下班
        if((control->shutdown == 1) && (control->task_count == 0)){
            pthread_mutex_unlock(&(control->lock));
            pthread_exit(NULL);
        }

        //开始干活
        task_t *task = control->queue_head;
        if(task != NULL){
            //控制器指向下一个任务
            control->queue_head = task->next;

            //如果下一个任务为空,则设置队尾也为空,防止空指针
            if(control->queue_head == NULL){
                control->queue_tail = NULL;
            }
            //任务数量-1
            control->task_count--;
        }

        //领完任务解锁,让下一个线程继续领任务
        pthread_mutex_unlock(&(control->lock));

        //执行任务
        if(task != NULL){
            (*(task->function))(task->arg);
            free(task);
        }
    }
    return NULL;
}

//创建线程池
threadpool_t* threadpool_creat(int thread_count){
    //分配控制器内存
    threadpool_t *control = (threadpool_t*)malloc(sizeof(threadpool_t)); //主控制

    //初始化控制器
    control->thread_count = thread_count;
    control->task_count = 0;
    control->shutdown = 0;
    control->queue_head = NULL;
    control->queue_tail = NULL;
    //初始化控制器变量
    pthread_mutex_init(&(control->lock),NULL);
    pthread_cond_init(&(control->notify),NULL);

    //为线程组分配内存
    control->threads = (pthread_t *)malloc(sizeof(pthread_t)*thread_count);

    //创建线程,等待任务
    for(int i = 0;i<control->thread_count;i++){
        pthread_create(&(control->threads[i]),NULL,worker_thread,(void *)control);
    }

    return control;
}

//提交任务到线程池
int threadpool_add(threadpool_t *control,void (*function)(void *),void *arg){
    //包装
    task_t *new_task = (task_t *)malloc(sizeof(task_t));
    new_task->function = function;
    new_task->arg = arg;
    new_task->next = NULL;

    //加锁放进队列
    pthread_mutex_lock(&(control->lock));

    //如果队列为空,头尾都为当下任务
    if(control->queue_head == NULL){
        control->queue_head = new_task;
        control->queue_tail = new_task;
    }else{
        //如果不为空,那么完善队列
        control->queue_tail->next = new_task;
        control->queue_tail = new_task;
    }
    //任务加加
    control->task_count++;

    //响铃,干活
    pthread_cond_signal(&(control->notify));

    pthread_mutex_unlock(&(control->lock));
    return 0;
}

//结束线程池
void threadpool_destroy(threadpool_t *control){
    if(control == NULL) return;

    //上锁,打烊
    pthread_mutex_lock(&(control->lock));
    control->shutdown = 1;

    //唤醒休眠的线程,检查标志
    pthread_cond_broadcast(&(control->notify));
    pthread_mutex_unlock(&(control->lock));

    //等待下班
    for(int i = 0;i<control->thread_count;i++){
        pthread_join(control->threads[i],NULL);
    }

    //拆除线程池
    free(control->threads);
    pthread_mutex_destroy(&(control->lock));
    pthread_cond_destroy(&(control->notify));
    free(control);
}

//---------------------------------------------------
//测试:矩阵计算
//---------------------------------------------------

#define MATRIX_SIZE 1000 // 1000x1000 的大矩阵
#define NUM_THREADS 8    // 线程池线程数

// 使用全局变量分配巨型矩阵，避免撑爆栈内存
long A[MATRIX_SIZE][MATRIX_SIZE];
long B[MATRIX_SIZE][MATRIX_SIZE];
long C[MATRIX_SIZE][MATRIX_SIZE]; // 结果矩阵

// 任务函数的参数结构体
typedef struct {
    int row;
} mat_task_arg_t;

// 具体的任务函数：计算结果矩阵的某一行
void compute_matrix_row(void *arg) {
    mat_task_arg_t *task_arg = (mat_task_arg_t *)arg;
    long r = task_arg->row;
    
    for (long j = 0; j < MATRIX_SIZE; j++) {
        C[r][j] = 0;
        for (long k = 0; k < MATRIX_SIZE; k++) {
            C[r][j] += A[r][k] * B[k][j];
        }
    }
    free(arg); // 必须由执行者释放参数内存
}

// ==========================================
// 主函数
// ==========================================
int main() {
    struct timeval start, end;

    // 1. 初始化矩阵数据
    printf("正在初始化 %dx%d 巨型矩阵...\n", MATRIX_SIZE, MATRIX_SIZE);
    for (long i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            A[i][j] = rand() % 10;
            B[i][j] = rand() % 10;
        }
    }

    printf("创建 %d 线程的线程池...\n", NUM_THREADS);
    threadpool_t *pool = threadpool_creat(NUM_THREADS);

    // 记录开始时间
    gettimeofday(&start, NULL);

    // 2. 发起任务洪流！将 1000 行计算任务塞入线程池
    for (long i = 0; i < MATRIX_SIZE; i++) {
        mat_task_arg_t *arg = (mat_task_arg_t *)malloc(sizeof(mat_task_arg_t));
        arg->row = i;
        threadpool_add(pool, compute_matrix_row, arg);
    }

    // 3. 销毁线程池，这会阻塞主线程，直到队列里的 1000 个任务全部被处理完
    threadpool_destroy(pool);

    // 记录结束时间
    gettimeofday(&end, NULL);
    long time_use = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
    
    printf("✅ 矩阵计算完毕！全部任务完美消化。\n");
    printf("⏱️ 总耗时: %f 秒\n", time_use / 1000000.0);

    return 0;
}
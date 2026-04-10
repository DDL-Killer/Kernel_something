#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>
#include<dirent.h>
#include<sys/stat.h>
#include<semaphore.h>
#include<unistd.h>

#define MAX_PATH_LEN 512
#define MAX_THREADS 100 //每个目录最多记录的子线程句柄数

//全局并发控制器
sem_t thread_sem; //信号量,用于控制同时访问某个资源的线程数量
pthread_mutex_t print_mutex; //互斥锁,用于保护共享资源的互斥访问

//搜索任务的配置结构体
typedef struct{
    char current_path[MAX_PATH_LEN]; //存储当前要搜索的目录路径
    char target_ext[16]; //要搜索的文件后缀名
    int max_depth; //最大搜索深度限制
    int current_depth; //当前目录的深度层级
}SearchTask;

//检查文件后缀的辅助函数
int ends_with(const char *str, const char *suffix){
    if(!str || !suffix) return 0;
    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);
    if(lensuffix > lenstr) return 0;
    return strncmp(str +lenstr -lensuffix,suffix,lensuffix) == 0;
}

//线程工作函数
void *search_dir(void *arg){
    SearchTask *task = (SearchTask*)arg;

    //超过最大深度,停止
    if(task->current_depth > task->max_depth){
        free(task);
        return NULL;
    }

    DIR *dir = opendir(task->current_path);
    if(!dir){
        //权限不足或无法打开静默跳过
        free(task);
        return NULL;
    }

    struct dirent *entry;
    pthread_t child_threads[MAX_THREADS];
    int thread_count = 0;

    while((entry = readdir(dir)) != NULL){
        //跳过"."".."
        if(entry->d_name[0] == '.') continue;

        char next_path[MAX_PATH_LEN];
        snprintf(next_path,sizeof(next_path),"%s/%s",task->current_path,entry->d_name);

        struct stat st;
        //获取文件信息
        if(lstat(next_path,&st) == -1) continue;

        if(S_ISDIR(st.st_mode)){
            //是目录,重新分配
            SearchTask *new_task = malloc(sizeof(SearchTask));
            strcpy(new_task->current_path,next_path);
            strcpy(new_task->target_ext,task->target_ext);
            new_task->max_depth = task->max_depth;
            new_task->current_depth = task->current_depth+1;

            //尝试获取并发令牌
            if(thread_count < MAX_THREADS && sem_trywait(&thread_sem) == 0){
                //获取成功,开启新线程并发处理
                if(pthread_create(&child_threads[thread_count],NULL,search_dir,new_task) == 0){
                    thread_count++;
                }else{
                    //创建失败,归还令牌并降级为同步执行
                    sem_post(&thread_sem);
                    search_dir(new_task);
                }
            }else{
                //令牌耗尽,亲自遍历
                search_dir(new_task);
            }
        }else if(S_ISREG(st.st_mode)){
            //普通文件,检查后缀
            if(ends_with(entry->d_name,task->target_ext)){
                //找到目标文件,加锁打印防止输出交错
                pthread_mutex_lock(&print_mutex);
                printf("[Found] %s\n",next_path);
                pthread_mutex_unlock(&print_mutex);
            }
        }
    }
    closedir(dir);

    //等待所有派生出的子线程完成
    for(int i = 0;i<thread_count ;i++){
        pthread_join(child_threads[i],NULL);
        sem_post(&thread_sem); //子线程结束,归还并发令牌
    }

    free(task);
    return NULL;
}

int main(int argc,char *argv[]){
    //假设最大并发为8

    int max_concurrency = 8;
    //初始化信号量和互斥锁
    sem_init(&thread_sem,0,max_concurrency-1); //扣除主线程本身
    pthread_mutex_init(&print_mutex,NULL);

    SearchTask *root_task = (SearchTask *)malloc(sizeof(SearchTask));
    strcpy(root_task->current_path,argc > 1 ? argv[1]:"."); //默认搜索当前目录
    strcpy(root_task->target_ext,".c");
    root_task->max_depth = 5;
    root_task->current_depth = 0;

    printf("开始搜索目录: %s, 目标文件: %s\n",root_task->current_path,root_task->target_ext);
    
    //启动根目录搜索
    search_dir(root_task);

    //清理资源
    sem_destroy(&thread_sem);
    pthread_mutex_destroy(&print_mutex);

    printf("搜索完成. \n");
    return 0;
}
#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<sys/time.h>
#include<string.h>

#define DATA_SIZE 100000000
#define THREAD_COUNT 16//根据cpu核心数调整

//传递给线程的参数结构体:把线程需要的多个数据打包为一个整体
typedef struct{
    int *sub_array;
    int size;
}ThreadArgs;

//比较函数
int compare(const void *a,const void *b){
    return (*(int*)a - *(int*)b);
}

//线程工作函数
void* thread_sort(void* arg){
    ThreadArgs *args = (ThreadArgs*)arg;
    //用标准快排对子数组排序
    qsort(args->sub_array,args->size,sizeof(int),compare);
    return NULL;
}

// //两路归并算法
// void merge(int left[],int left_len,int right[],int right_len,int result[]){
//     int i = 0,j = 0,k = 0;
//     while(i<left_len&&j<right_len){
//         if(left[i]<=right[j]){
//             result[k++] = left[i++];
//         }else{
//             result[k++] = right[j++];
//         }
//     }
//     while(i<left_len) result[k++] = left[i++];
//     while(j<right_len) result[k++] = right[j++];
// }

//实际使用的mergr算法
//合并两个相邻的有序区间:data[left...mid] data[mid+1,right]
void merge(int *data,int left,int mid,int right,int *temp){
    int i = left;
    int j = mid + 1;
    int k = left;

    while(i<=mid && j<=right){
        if(data[i]<=data[j]){ temp[k++] = data[i++];}
        else{ temp[k++] = data[j++];}
    }

    while(i<=mid) temp[k++] = data[i++];
    while(j<=right) temp[k++] = data[j++];
    
    for(int p = left;p<=right;p++){
        data[p] = temp[p];
    }

}

int main(){
    int *data = malloc(sizeof(int)*DATA_SIZE);
    int *result = malloc(sizeof(int)*DATA_SIZE);

    srand(time(NULL));
    for(int i = 0;i<DATA_SIZE;i++){
        data[i] = rand()%100000;
    }

    //记录时间
    struct timeval start,end;
    gettimeofday(&start,NULL);

    //切割数组创建线程
    pthread_t threads[THREAD_COUNT]; //线程ID数组
    ThreadArgs args[THREAD_COUNT]; //参数结构体数组
    int sub_size = DATA_SIZE / THREAD_COUNT; //每块基础大小

    for(int i = 0;i<THREAD_COUNT;i++){
        args[i].sub_array = &data[i*sub_size]; //计算第i个线程要开始处理的位置
        args[i].size = (i == THREAD_COUNT -1)? (DATA_SIZE-i*sub_size) : sub_size; //计算第i个线程要处理的元素个数,三元运算符解决不被整除的情况
        
        if(pthread_create(&threads[i],NULL,thread_sort,&args[i])!=0){
            perror("Failed to create thread");
            return 1;
        }
    }

    //等待所有线程完成(Join)
    for(int i = 0;i<THREAD_COUNT;i++){
        pthread_join(threads[i],NULL);
    }

    //合并已排序的数组
    //申请临时数组用于归并
    int *temp = malloc(sizeof(int) * DATA_SIZE);
    if(temp == NULL){
        perror("Failed to allocate memory for temp array");
        return 1;
    }

    //current_merged_size 记录当前已经合并好的长度
    int current_merged_size = args[0].size;

    //合并数uz
    for(int i = 1;i<THREAD_COUNT;i++){
        int left = 0; //合并的起点是0
        int mid = current_merged_size - 1; //合并的终点
        int right = current_merged_size + args[i].size -1;  //加上当前处理后的终点

        //合并
        merge(data,left,mid,right,temp);
        
        //更新合并总长度
        current_merged_size += args[i].size;
    }

    gettimeofday(&end,NULL);

    long time_use = (end.tv_sec-start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
    printf("多线程排序耗时: %ld 微秒\n",time_use);

    free(data);
    free(temp);
    return 0;
}
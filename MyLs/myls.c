#include<stdio.h>
#include<getopt.h>
#include<unistd.h>
#include<stdlib.h>
#include<dirent.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<time.h>

typedef struct{
    char name[256];
    char full_path[1024]; //完整路径
    struct stat info; //
}FileInfo;

//比较函数的声明
int cmp_name(const void *a,const void *b);
int cmp_time(const void *a,const void *b);

//第三步:位运算
#define FLAG_a (1 << 0)
#define FLAG_l (1 << 1)
#define FLAG_R (1 << 2)
#define FLAG_t (1 << 3)
#define FLAG_r (1 << 4)
#define FLAG_i (1 << 5)
#define FLAG_s (1 << 6)

int g_flags = 0;

void do_ls(const char *path);

int main(int argc,char *argv[]){
    //第一步
    int opt;
    while((opt = getopt(argc,argv,"alRtris")) != -1){
        switch (opt){
        case 'a':g_flags |= FLAG_a; break;
        case 'l':g_flags |= FLAG_l; break;
        case 'R':g_flags |= FLAG_R; break;
        case 't':g_flags |= FLAG_t; break;
        case 'r':g_flags |= FLAG_r; break;
        case 'i':g_flags |= FLAG_i; break;
        case 's':g_flags |= FLAG_s; break;
        default:
            fprintf(stderr,"Usage: %s [-alRtris][file...]\n",argv[0]);
            exit(EXIT_FAILURE);
        }
    }
    
    //用optind解析参数
    if(optind == argc){
        do_ls(".");
    }else{
        for(int i = optind;i<argc;i++){
            if(argc - optind > 1) printf("%s:\n",argv[i]);
            do_ls(argv[i]);
            if(i < argc - 1) printf("\n"); // 目录之间空一行
        }
    }
    return 0;
}

// void do_ls(const char *path){
//     DIR *dir = opendir(path);
//     if(dir == NULL){
//         perror(path);
//         return;
//     }

//     struct dirent *entry;
//     while((entry = readdir(dir))!=NULL){
//         //如果没有-a且文件名为.时,跳过
//         if(!(g_flags & FLAG_a) && entry->d_name[0] == '.'){
//             continue;
//         }
//         printf("%s  ",entry->d_name);
//     }
//     printf("\n");
//     closedir(dir);
// }

void do_ls(const char *path){
    DIR *dir = opendir(path);
    if(dir == NULL){
        perror(path);
        return;
    }

    struct dirent *entry;

    int capacity = 10; //初始
    int count = 0; //当前文件数量
    FileInfo *files = (FileInfo *)malloc(capacity * sizeof(FileInfo));

    while((entry = readdir(dir))!=NULL){
        if(!(g_flags & FLAG_a) && entry->d_name[0] == '.'){
            continue;
        }
        //动态扩容
        if(count >= capacity){
            capacity *= 2;
            files = (FileInfo *)realloc(files,capacity*sizeof(capacity));
        }
        //存文件名
        strcpy(files[count].name,entry->d_name);

        if(strcmp(path,".")==0){
            sprintf(files[count].full_path,"%s",entry->d_name);
        }else if(path[strlen(path)-1]=='/'){
            sprintf(files[count].full_path,"%s%s",path,entry->d_name);
        }else{
            sprintf(files[count].full_path,"%s/%s",path,entry->d_name);
        }

        //获取文件详情
        if(lstat(files[count].full_path,&files[count].info)==-1){
            perror("lstat");
        }

        count++;
    }
    closedir(dir);

    if(g_flags & FLAG_t){
        qsort(files,count,sizeof(FileInfo),cmp_time);
    }else{
        qsort(files,count,sizeof(FileInfo),cmp_name);
    }

    for(int i = 0;i < count;i++){
        //处理 -i (index)
        if(g_flags & FLAG_i){
            printf("%lu ",files[i].info.st_ino);
        }

        //处理 -s (blocks)
        //ls -s 显示是KB单位,st_blocks是512字节块,所以 /2
        if(g_flags & FLAG_s){
            printf("%4lld ",(long long)files[i].info.st_blocks);
        }

        printf("%s ",files[i].name);
    }
    printf("\n");

    free(files);
}
//按文件名排序
int cmp_name(const void *a,const void *b){
    FileInfo *fa = (FileInfo *)a;
    FileInfo *fb = (FileInfo *)b;

    int result = strcmp(fa->name,fb->name);

    //解决 -r
    if(g_flags & FLAG_R){
        return -result;
    }
    return result;
}
//按时间排序
int cmp_time(const void *a,const void *b){
    FileInfo *fa = (FileInfo *)a;
    FileInfo *fb = (FileInfo *)b;

    //最新的时间在前
    long result = fb->info.st_mtime-fa->info.st_mtime;
    if(result == 0){
        return cmp_name(a,b);
    }
    
    //解决 -r
    if(g_flags & FLAG_r){
        return -result;
    }

    return (int)result;
}
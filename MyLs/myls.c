#include<stdio.h>
#include<getopt.h>
#include<unistd.h>
#include<stdlib.h>
#include<dirent.h>
#include<string.h>

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

void do_ls(const char *path){
    DIR *dir = opendir(path);
    if(dir == NULL){
        perror(path);
        return;
    }

    struct dirent *entry;
    while((entry = readdir(dir))!=NULL){
        //如果没有-a且文件名为.时,跳过
        if(!(g_flags & FLAG_a) && entry->d_name[0] == '.'){
            continue;
        }
        printf("%s  ",entry->d_name);
    }
    printf("\n");
    closedir(dir);
}
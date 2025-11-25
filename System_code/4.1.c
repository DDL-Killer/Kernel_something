//获取其命令行参数，然后针对每一个命令行参数打印其文件类型
#include<stdio.h>
#include<sys/stat.h>
#include<stdlib.h>

int main(int argc,char *argv[]){
    int i;
    struct stat buf;
    char *ptr;
    for(i = 1;i<argc;i++){
        printf("%s: ",argv[i]);
        if(lstat(argv[i],&buf)<0){
            printf("lstat error");
            continue;
        }
    }
    if(S_ISREG(buf.st_mode))
        ptr = "regular";
}
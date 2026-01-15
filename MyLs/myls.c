#include<stdio.h>
#include<getopt.h>
#include<unistd.h>
#include<stdlib.h>
#include<dirent.h>

int main(int argc,char *argv[]){
    //第一步
    int opt;
    while((opt = getopt(argc,argv,"a")) != -1){
        switch (opt)
        {
        case 'a':
            printf("There is a\n");
            break;
        default:
            exit(EXIT_FAILURE);
        }
    }
    printf("finished,let's begin!\n");
    //第二步
    DIR *dir = opendir(".");
    if(!dir){
        perror("opendir");
        return 1;
    }

    struct dirent *entry;
    while((entry = readdir(dir))!=NULL){
        printf("%s\n",entry->d_name);
    }

    closedir(dir);
    return 0;
}
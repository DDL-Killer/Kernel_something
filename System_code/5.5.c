#include<sys/wait.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
int main(){
    int aa=fork();
    if(aa==0){
        printf("child process\n");
        printf("pid : %d\n",getpid());
        exit(100);
    }
    else{
        int bb ;
        int cc = wait(&bb);
        printf("pid : %d\n",getpid());
        printf("parent process\n");
        printf("pid : %d\n",getpid());
        printf("Parent :Child exit is %d\n",WEXITSTATUS(bb));
        printf("pid : %d\n",getpid());
    }
}
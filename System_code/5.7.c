#include<stdio.h>
#include<unistd.h>
#include<sys/wait.h>
int main(){
    int aa = fork();
    printf("pid : %d\n",getpid());
    if(aa==0){
        close(STDOUT_FILENO);
        printf("pid : %d\n",getpid());
        printf("Who cares?");
        printf("pid : %d\n",getpid());
    }else{
        wait(NULL);
    }
}
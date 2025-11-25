#include<stdio.h>
#include<unistd.h>
#include<sys/wait.h>
#include<string.h>
#include<fcntl.h>

int main(){
    int x = 100;
    int rc = fork();
    if(rc==0){
        printf("Child x: %d (pid = %d)\n",x,getpid());
        x=200;
        printf("Changed x to %d (pid = %d)\n ",x,getpid());
    }else{
        printf("Parent : %d (pid = %d)\n",x,getpid());
        x=300;
        printf("Changed x to %d (pid = %d)\n",x,getpid());
        int wc = wait(NULL);
    }
}
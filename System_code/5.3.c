#include<stdio.h>
#include<unistd.h>
int main(){
    int pipefd[2];
    pipe(pipefd);

    int cc = fork();
    if(cc==0){
        close(pipefd[0]);
        printf("hello  pid:%d\n",getpid());
        write(pipefd[1], "done", 5);
        printf("pid : %d\n",getpid());
        close(pipefd[1]);
        printf("pid : %d\n",getpid());
    }else{
        close(pipefd[1]);
        printf("pid : %d\n",getpid());
        char buf[100];
        read(pipefd[0], buf, 5);
        printf("pid : %d\n",getpid());
        printf("Goodbye pid : %d\n",getpid());
        close(pipefd[0]);
        printf("pid : %d\n",getpid());
    }
}
#include<stdio.h>
#include<unistd.h>
#include<sys/wait.h>
#include<stdlib.h>
int main(){
    int fdpipe[2];


    pipe(fdpipe);
    int aa = fork();

    int bb = fork();


    if(aa==0){


        close(fdpipe[0]);


        dup2(fdpipe[1],STDOUT_FILENO);

        printf("Hello i am chlid 1\n");
        fflush(stdout);

        close(fdpipe[1]);

        return 0;
    }
    if(bb==0){


        close(fdpipe[1]);


        dup2(fdpipe[0],STDIN_FILENO);
        char buf[1024];
        int cc = read(STDIN_FILENO,buf,sizeof(buf));


        printf("Here: ");
        fflush(stdout);
        
        write(STDOUT_FILENO,buf,cc);
        close(fdpipe[0]);
        return 0;
    }
    wait(NULL);
    wait(NULL);
}
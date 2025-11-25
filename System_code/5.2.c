#include<fcntl.h>
#include<unistd.h>
#include<sys/wait.h>
int main(){
    int fd = open("11.txt",O_WRONLY|O_CREAT|O_TRUNC,0644);
    int cc = fork();
    if(cc==0){
        write(fd,"child\n",6);
        close(fd);
    }
    else{
        wait(NULL);
        write(fd,"parent\n",7);
        close(fd);
    }
}
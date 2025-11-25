#include<stdio.h>
#include<fcntl.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<string.h>


int main(){
    int fd;
    ssize_t byte_read,bytes_written;
    const char *msg = "hello";
    char buffer[20];

    if((fd= open("tt",O_RDWR))==-1){
       perror("open");
       exit(1);
    }

    lseek(fd,-19999,SEEK_END);

    if((bytes_written = write(fd,msg,strlen(msg))) == -1){
        perror("write");
        exit(1);
    }

    printf("Successfully %ld bytes\n", bytes_written);
    // if((byte_read = read(fd,buffer,sizeof(buffer))) == -1){
    //    perror("read");
    //    exit(1);
    // }

    // write(STDOUT_FILENO,buffer,byte_read);
    close(fd);
    return 0;
}
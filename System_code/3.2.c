#include<unistd.h>
#include<fcntl.h>
#include<stdio.h>
#include<stdlib.h>

char buf1[] = "abcdefg";
char buf2[] = "ABCDEFG";

int main(){
    int fd;
    if((fd = creat("file.hole",O_RDWR))<0){
        printf("creat error");
    }
    if(write(fd,buf1,7)!= 7){
        printf("buf1 write error");
    }
    if(lseek(fd,16383,SEEK_SET) == -1){
        printf("lseek error");
    }
    if(write(fd,buf2,7)!=7){
        printf("buf2 write error");
    }
    exit(0);
}
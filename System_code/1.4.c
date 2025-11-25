#include<unistd.h>
#include<stdlib.h>
#define BUFFSIZE 4096

int main(){
    int n;
    char buf[BUFFSIZE];

    while((n = read(STDIN_FILENO,buf,BUFFSIZE))>0){
        write(STDOUT_FILENO,buf,n);
    }
    exit(0);
}

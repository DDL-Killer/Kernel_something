#include<fcntl.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define BUFFER_SIZE 4096

//dfdf

int main(int argc,char *argv[]){
    int append_mode=0;
    char **files = NULL;
    int num_files = 0;
    for(int i = 1;i<argc;i++){
        if(strcmp(argv[i],"-a")==0){
            append_mode = 1;
        }else{
            files = &argv[i];
            num_files = argc-i;
            break;
        }
    }

    if(num_files==0){
        fprintf(stderr,"Usage: %s [-a] file...\n",argv[0]);
        exit(EXIT_FAILURE);
    }

    //存储所有打开文件的描述符（每个文件对应一个int型的文件描述符）
    int *fds = malloc(num_files*sizeof(int));
    for(int i = 0;i<num_files;i++){
        int flags = O_WRONLY|O_CREAT;
        flags |= (append_mode)?O_APPEND:O_TRUNC;

        if((fds[i]=open(files[i],flags,0644))==-1){
            perror("open");
            exit(EXIT_FAILURE);
        }
    }
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    while((bytes_read=read(STDIN_FILENO,buffer,BUFFER_SIZE))){
        if(write(STDOUT_FILENO,buffer,bytes_read)!=bytes_read){
            perror("write stdout");
            exit(EXIT_FAILURE);
        }

    for(int i = 0;i<num_files;i++){
        if(write(fds[i],buffer,bytes_read)!=bytes_read){
            perror("write file");
            exit(EXIT_FAILURE);
        }
    }
}

    if(bytes_read==-1){
         perror("read");
         exit(EXIT_FAILURE);
    }
    for (int i = 0;i<num_files;i++){
        close(fds[i]);
    }
free(fds);
return 0;
}
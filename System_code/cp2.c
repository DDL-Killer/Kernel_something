#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<getopt.h>

#define _GNU_SOURCE
#define BUFFERSIZE 4096
#define COPYMODE 0644

void oops(char *,char *);

int main(int ac, char *av[])
{
    int in_fd,out_fd,n_chars;
    char buf[BUFFERSIZE];
    int interactive = 0;
    int opt;

    while((opt = getopt(ac,av,"i")) != -1)
       switch(opt){
        case 'i':
            interactive = 1;
            break;
        case '?':
            fprintf(stderr,"Unknown option : -%c\n",opt);
            fprintf(stderr,"usage: %s [-i] source destination",av[0]);
            exit(1);
       }
    
    if(ac-optind!=2){
        fprintf(stderr,"usage: %s [-i] source destination\n",av[0]);
        exit(1);
    }

    char *source = av[optind];
    char *dest = av[optind+1];
    
    if((in_fd = open(source,O_RDONLY)) == -1){
        oops("Cannot open",source);
    }

    //检查目标文件是否存在
    if(access(dest,F_OK) == 0){
        if(interactive){
            printf("%s:overwrite '%s'?(y/n)\n",av[0],dest);
            fflush(stdout);
            char response[10];
            if(fgets(response,sizeof(response),stdin) == NULL){
                oops("Error reading response","");
            }
            if(response[0] != 'y' && response[0] != 'Y'){
                printf("not overwritten\n");
                close(in_fd);
                exit(0);
            }
        }
    }

    //打开目标文件，如果存在会被截断
    if((out_fd = creat(dest,COPYMODE)) == -1){
        oops("Cannot creat",dest);
    }

    while ((n_chars = read(in_fd,buf,BUFFERSIZE))>0){
        if(write(out_fd,buf,n_chars) != n_chars){
            oops("Write error to",dest);
        }
    }


    if(n_chars == -1){
        oops("Read error from",av[1]);
    }

    if(close(in_fd) == -1 || close(out_fd) == -1){
        oops("Error closing file","");
    }
    return 0;
}

void oops(char *s1,char *s2){
    fprintf(stderr,"Error: %s",s1);
    perror(s2);
    exit(1);
}

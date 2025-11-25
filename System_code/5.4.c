#include<unistd.h>
#include<sys/wait.h>
int main(){
    char *argv[]={"ls",NULL};
    extern char **environ;
    int aa = fork();
    if(aa==0){
        execl("/bin/ls","ls",NULL);
        _exit(1);
    }
    int bb = wait(NULL);
    int cc = fork();
    if(cc==0){
        execvp("ls",argv);
        _exit(1);
    }
    int dd = wait(NULL);
    int ee = fork();
    if(ee==0){
        execle("/bin/ls","ls",NULL,environ);
        _exit(1);

    }
}
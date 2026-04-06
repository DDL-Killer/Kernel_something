#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/wait.h>
#include<signal.h>
#include<fcntl.h>

#define MAX_CMD_LEN 1024
#define MAX_ARGS 64

//解决重定向输出到终端后没有换行直接打印终端信息
//使用全局变量
int g_need_newline = 0;

//循环函数打印shell信息
void print_prompt(){
    char cwd[1024];

    //判断是不是要换行
    if(g_need_newline){
        printf("\n");
        g_need_newline = 0;//重置
    }

    if(getcwd(cwd,sizeof(cwd))!=NULL){
        printf("\033[1;32mlyh@super-shell\033[0m:\033[1;34m%s\033[0m$ ",cwd);
    }else{
        perror("getcwd error");
    }

    fflush(stdout);
}

//字符串解析
void parse_command(char *input,char **args){
    int i = 0;
    //去除换行符
    input[strcspn(input,"\n")] = 0;

    char *token = strtok(input," \t");
    while(token != NULL){
        args[i++] = token;
        token = strtok(NULL, " \t");
    }
    args[i] = NULL;
}

//进程分裂,用fork()克隆一个子进程,让shell主进程等待,子进程替换为命令执行
//增加is_background参数
void execute_external_command(char **args,int is_background){
    pid_t pid= fork();

    if(pid < 0){
        perror("fork failed");
    }else if(pid == 0){
        //解决重定向
        int fd;

        //遍历寻找重定向符号
        for(int i = 0;args[i]!=NULL;i++){

            //处理输出覆盖重定向 " > "
            if(strcmp(args[i],">") == 0){
                //O_CREAT:不存在就创建 O_WRONLY:只写 O_TRUNC:存在就清空 0644:读写 读 读
                fd = open(args[i+1],O_CREAT|O_WRONLY|O_TRUNC,0644);
                if(fd<0) {perror("open >");exit(1);}

                dup2(fd,STDOUT_FILENO);//标准输出
                close(fd);

                args[i] = NULL;//截断命令行,不让execvp获取>之后的内容
                break;
            }
            //处理输出追加重定向 " >> "
            else if(strcmp(args[i],">>") == 0){
                //O_APPEND:追加
                fd = open(args[i+1],O_CREAT|O_WRONLY|O_APPEND,0644);
                if(fd<0) {perror("open >>");exit(1);}

                dup2(fd,STDOUT_FILENO);
                close(fd);
                args[i] = NULL;
                break;
            }
            //处理输入重定向 " < "
            else if(strcmp(args[i],"<") == 0){
                //O_RDONLY:只读
                fd = open(args[i+1],O_RDONLY);
                if(fd<0) {perror("open <");exit(1);}

                dup2(fd,STDIN_FILENO);
                close(fd);
                args[i] = NULL;
                break;
            }
        }
        //子进程
        //execvp 会在环境变量PATH中寻找args[0]
        if(execvp(args[0],args) == -1){
            perror("Command not found");
            exit(EXIT_FAILURE);//如果命令为空,子进程自杀
        }
    }else{
        //父进程
        if(is_background){
            printf("[Process running in background,PID: %d]\n",pid);
        }else{
            int status;
            waitpid(pid,&status,0);//阻塞等待子进程结束
        }
    }
}

//拦截内置命令,如cd
int execute_builtin(char **args){
    if(args[0] == NULL) return 1;//空

    //exit
    if(strcmp(args[0],"exit") == 0){
        printf("Bye~ Bye~,Lyh!\n");
        exit(0);
    }

    //cd
    if(strcmp(args[0],"cd") == 0){
        if(args[1] == NULL){
            fprintf(stderr,"cd: no input\n");
            return 1;
        }

        //存储当前位置
        char current_pwd[1024];
        if(getcwd(current_pwd,sizeof(current_pwd)) == NULL){
            perror("getcwd");
            return 1;
        }

        const char *target_dir = args[1];
        int is_cd_dash = 0;

        //判断是不是cd -
        if(strcmp(args[1],"-") == 0){
            target_dir = getenv("OLDPWD");
            if(target_dir == NULL){
                fprintf(stderr,"cd: OLDPWD is NULL\n");
                return 1;
            }
            is_cd_dash = 1; //标记
        }

        //切换目录
        if(chdir(target_dir) == 0){
            //切换ok 更新环境变量

            //把切换前的环境变量保存为OLDPWD,非0覆盖
            setenv("OLDPWD",current_pwd,1);
            
            //获取切换后的新路径,写入PWD
            char new_pwd[1024];
            if(getcwd(new_pwd,sizeof(new_pwd)) != NULL){
                setenv("PWD",new_pwd,1);
            }

            //如果是cd - 打印目录
            if(is_cd_dash){
                printf("%s\n",target_dir);
            }
        }else{
            //切换失败
            perror("cd");
        }
        return 1;
    }
    return 0;
}

//子进程死掉的,父进程如果不去用wait回收,就变成僵尸进程,占用系统进程表
//在子进程死亡,内核给父进程发送SIGCHLD信号
void handle_sigchld(int sig){
    //WNOHANG 非阻塞,如果没有僵尸进程,立即返回
    //使用while防止同时死去多个子进程
    while(waitpid(-1,NULL,WNOHANG)>0){
        //静默回收
    }
}

//信号处理:屏蔽 ctrl+c
int main(){
    char input[MAX_CMD_LEN];
    char *args[MAX_ARGS];

    //解决僵尸进程
    signal(SIGCHLD,handle_sigchld);

    //忽略ctrl+c
    signal(SIGINT,SIG_IGN);

    while(1){
        print_prompt();

        if(fgets(input,sizeof(input),stdin) == NULL){
            printf("\n");  //处理Ctrl+D
            break;
        }

        parse_command(input,args);

        //如果回车,跳过
        if(args[0] == NULL){
            continue;
        }


        //在执行命令前,检查args数组的最后一个有效元素是不是&
        //如果是,删除设置为NULL,并立一个is_background = 1的标志
        int is_background = 0;
        int i = 1;
        //找到最后一个命令块是第几个命令块
        while(args[i] != NULL) i++;
        //如果倒数第二个(也就是输入命令块的最后一个,因为parse_command()最后一个while()会i++,然后把这个新的i++变成了NULL)
        if(i > 0 && strcmp(args[i-1],"&") == 0 ){
            is_background = 1;
            args[i-1] = NULL;
        }

        //尝试内部指令执行
        if(execute_builtin(args)){
            //内部命令执行后,标记需要换行
            //排除 cd -
            if(!(strcmp(args[0],"cd")==0 && strcmp(args[1],"-")==0)){
                g_need_newline = 1;
            }
            continue;
        }

        //外部指令执行
        execute_external_command(args,is_background);

        //如果是前台进程,设置换行,后台进程不影响
        if(!is_background){
            g_need_newline = 1;
        }
    }
    return 0;
}
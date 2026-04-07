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
//抽离出来的重定向函数:只在子进程(pid==0)中调用
void handle_redirection(char **args){
    //从之前execute_external_command中移出来
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
}
//进程分裂,用fork()克隆一个子进程,让shell主进程等待,子进程替换为命令执行
//增加is_background参数
void execute_external_command(char **args,int is_background){
    pid_t pid= fork();

    if(pid < 0){
        perror("fork failed");
    }else if(pid == 0){
        //子进程恢复默认信号处理,防止后台进程杀不死
        signal(SIGINT,SIG_DFL);
        //解决重定向
        //抽离后直接调用
        handle_redirection(args);
        
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

//实现多级管道,增加解析命令行逻辑,先前是" \t"切割,现在先按照"|"切分,再按照之前的逻辑切分
//需要循环来处理,如果有N个命令,需要N-1个管道
//对于第i个命令:
    //如果不是第一个,输入来自前一个管道的读端
    //如果不是最后一个,输出向当前管道的写端
//实现多级管道的通用函数
void execute_pipeline(char *input,int is_background){
    //命令块数量
    int num_cmds = 0;
    char *cmd_pieces[MAX_ARGS];

    //按照管道拆分
    char *token = strtok(input,"|");
    while(token != NULL){
        cmd_pieces[num_cmds++] = token;
        token = strtok(NULL,"|");
    }
    //创建管道
    int pipefds[2 * (num_cmds - 1)];
    for(int i = 0;i<num_cmds-1;i++){
        if(pipe(pipefds + i *2)<0){
            perror("pipe");
            return;
        }
    }

    pid_t last_pid;

    for(int i = 0;i<num_cmds;i++){
        pid_t pid = fork();
        if(pid == 0){
            //如果不是第一个命令,从上一个管道读
            if(i>0){
                dup2(pipefds[(i-1)*2],STDIN_FILENO);
            }
            //如果不是最后一个命令,往当前管道写
            if(i<num_cmds - 1){
                dup2(pipefds[i*2+1],STDOUT_FILENO);
            }

            //关闭所有管道描述符(子进程副本)
            for(int j = 0;j<2*(num_cmds-1);j++){
                close(pipefds[j]);
            }

            //解析当前命令块参数
            char *args[MAX_ARGS];
            parse_command(cmd_pieces[i],args);

            //处理块内的重定向
            //使用抽离出的新函数
            handle_redirection(args);

            execvp(args[0],args);
            perror("execvp failed");
            exit(1);
        }else if(pid<0){
            perror("fork in pipeline failed");
        }

        //记录管道最后一个进程的PID
        if(i == num_cmds -1){
            last_pid = pid;
        }
    }
    //父进程关闭所有的管道描述符
    for(int i = 0;i<2*(num_cmds-1);i++){
        close(pipefds[i]);
    }

    //父进程等待策略
    if(is_background){
        printf("[Pipeline running in background, last PID: %d]\n", last_pid);
    }else{
        //等待所有子进程
        for(int i = 0;i<num_cmds;i++){
            wait(NULL);
        }
    }
}

//信号处理:屏蔽 ctrl+c
int main(){
    char input[MAX_CMD_LEN];
    char input_copy[MAX_CMD_LEN];//备份
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

        //去除尾部换行符
        input[strcspn(input,"\n")] = 0;
        if(strlen(input) == 0) continue;//如果输入换行,继续循环

        int is_background = 0;
        int ptr = strlen(input)-1;
        //从后往前跳空格
        while(ptr>=0 && (input[ptr] == ' ' || input[ptr] == '\t')){ptr--;}

        //如果最后一个非空字符是 & ,立FLAG并截断
        if(ptr >= 0 && input[ptr] == '&'){
            is_background = 1;
            input[ptr] = '\0';
        }

        //备份input
        strcpy(input_copy,input);

        //路由出发,检查是否有管道
        if(strchr(input_copy,'|') != NULL){
            //走管道
            execute_pipeline(input_copy,is_background);
            if(!is_background) g_need_newline = 1;
        }else{
            //走普通
            parse_command(input_copy,args);
            //如果回车,跳过
            if(args[0] == NULL) continue;
            //尝试内部指令执行
            if(execute_builtin(args)){
                //内部命令执行后,标记需要换行
                //排除 cd -
                if(!(strcmp(args[0],"cd")==0 && strcmp(args[1],"-")==0)) g_need_newline = 1;
                continue;
            }

            //外部指令执行
            execute_external_command(args,is_background);

            //如果是前台进程,设置换行,后台进程不影响
            if(!is_background) g_need_newline = 1;
        }
    }
    return 0;
}
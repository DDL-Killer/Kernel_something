#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/wait.h>
#include<signal.h>

#define MAX_CMD_LEN 1024
#define MAX_ARGS 64

//循环函数打印shell信息
void print_prompt(){
    char cwd[1024];
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
void execute_external_command(char **args){
    pid_t pid= fork();

    if(pid < 0){
        perror("fork failed");
    }else if(pid == 0){
        //子进程
        //execvp 会在环境变量PATH中寻找args[0]
        if(execvp(args[0],args) == -1){
            perror("Command not found");
            exit(EXIT_FAILURE);//如果命令为空,子进程自杀
        }
    }else{
        //父进程
        int status;
        waitpid(pid,&status,0);//阻塞等待子进程结束
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

//信号处理:屏蔽 ctrl+c
int main(){
    char input[MAX_CMD_LEN];
    char *args[MAX_ARGS];

    //忽略ctrl+c
    signal(SIGINT,SIG_IGN);

    while(1){
        print_prompt();

        if(fgets(input,sizeof(input),stdin) == NULL){
            printf("\n");  //处理Crul+D
            break;
        }

        parse_command(input,args);

        //如果回车,跳过
        if(args[0] == NULL){
            continue;
        }

        //尝试内部指令执行
        if(execute_builtin(args)){
            continue;
        }

        //外部指令执行
        execute_external_command(args);
    }
    return 0;
}
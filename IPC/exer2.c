#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdlib.h>

int main()
{
    int pipefd[2];  // 定义管道文件描述符数组，fd[0]用于读取管道，fd[1]用于写入管道
    int ret;
    ret = pipe(pipefd);   // 创建管道

    if(ret == -1)
    {   // 管道创建失败
        perror("pipe error");
        exit(EXIT_FAILURE);
    }

    int i = 0;
    pid_t pid, wpid;    // 定义进程描述符

    // 创建两个子进程
    // do
    // {
    //     pid = fork();   
    // } while (pid > 0 && i++ < 2);

    for(i = 0; i < 2; ++i)
    {
        pid = fork();
        if(pid == 0)
            break;
    }

    if(i == 2) // 父进程，只有父进程才会执行到i == 2
    {
        int status;
        close(pipefd[0]);  // 关闭父进程的读端
        close(pipefd[1]);  // 关闭父进程的写端
        wpid = wait(&status);
        printf("child %d process exit with %d\n", wpid, status);
        wpid = wait(&status);
        printf("child %d process exit with %d\n", wpid, status);
    }
    else if(i == 0) // 创建的第一个子进程
    {
        sleep(1);
        // 子进程1--写管道
        close(pipefd[0]);  // 关闭子进程1的读端
        dup2(pipefd[1], STDOUT_FILENO); // 将管道的写端重定向到标准输出
        execlp("ls", "ls", "-l", NULL); // 执行ls -l命令
    }
    else if(i == 1) // 创建的第二个子进程
    {
        sleep(1);
        // 子进程2--读管道
        close(pipefd[1]);  // 关闭子进程2的写端
        dup2(pipefd[0], STDIN_FILENO); // 将管道的读端重定向到标准输入
        execlp("wc", "wc", "-l", NULL); // 执行wc -l命令
    }
    return 0;
}
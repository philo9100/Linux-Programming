#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>


int main()
{
    pid_t pid;
    pid = fork();

    if(pid == -1)
    {
        perror("Fork Failed");
        exit(EXIT_FAILURE);
    }
    else if(pid == 0) // 子进程
    {
        printf("This is child process, pid = %d, I am going to stop myself\n", getpid());
        raise(SIGSTOP); // 子进程暂停自己
        sleep(1);
        while(1)
        {
            printf("This is child process, pid = %d, I am alive!\n", getpid());
            sleep(1);
        }
    }
    else if(pid > 0) // 父进程
    {
        sleep(2);
        printf("This is parent process, pid = %d, I launch child process\n", getpid());
        kill(pid, SIGCONT); // 父进程向子进程发送SIGCONT信号，使子进程继续运行
        sleep(5);
        kill(pid, SIGKILL); // 父进程向子进程发送SIGKILL信号，使子进程终止
        wait(NULL); // 等待子进程结束
    }
    return 0;
}
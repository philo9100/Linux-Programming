#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main()
{
    pid_t pid; // 声明进程标识符
    int i;

    for(i = 0; i < 5; ++i) // 循环创建5个子进程，避免子进程创建子进程要进行判断
    {
        if((pid = fork()) == 0) // 如果当前是子进程执行fork就退出循环 
        {
            break;
        }
    }
    if(pid == -1)
    {
        perror("Fork Faild");
        exit(EXIT_FAILURE);
    }
    else if(pid > 0)
    {
        sleep(5);
        printf("This is parent process, pid = %d\n", getpid());
    }
    else if(pid == 0)
    {
        sleep(i);
        printf("This is %d child process, pid = %d\n", i+1, getpid());
    }
    return 0;
}
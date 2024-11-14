#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

// 信号处理函数
void signal_handler(int signo)
{
    printf("Caught the SIGINT signal\n");
}

int main()
{
    // 注册信号处理函数
    if(signal(SIGINT, signal_handler) == SIG_ERR)
    {
        perror("Signal Failed");
        exit(EXIT_FAILURE);
    }

    while(1)
    {
        printf("Try pressing Ctrl+C\n");
        printf("This is parent process, pid = %d\n", getpid());
        sleep(1);
    }
    /*
    在bash窗口循环输出信息，直到按下Ctrl+C，执行信号处理函数，然后继续循环输出信息
    可以按下Ctrl+Z，暂停程序，通过jobs命令查看当前被挂起的进程的jobid，然后在bash窗口输入fg %1 (bg %jobid 在后台运行)，继续执行程序  
    在程序暂停时，使用kill命令杀死进程，会看到暂停中的进程为被杀死，但是在程序继续运行时，会看到程序被杀死      
    */

    return 0;
}



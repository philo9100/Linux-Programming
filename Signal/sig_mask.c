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
    sigset_t new_set, old_set, pending_set;

    sigemptyset(&new_set);  // 将自定义信号集清空，即将所有位设置为0
    sigaddset(&new_set, SIGINT);    // 将 SIGINT 信号添加到信号集中

    // 判断 SIGINT 信号是否在自定义信号集中，若在返回1，否则返回0
    if(sigismember(&new_set, SIGINT) == 1)  
    {
        printf("SIGINT is in new_set\n");
    }
    else
    {
        printf("SIGINT is not in new_set\n");
    }

    // 注册信号处理函数
    if(signal(SIGINT, signal_handler) == SIG_ERR) 
    {
        perror("Signal Failed");
        exit(EXIT_FAILURE);
    }

    // 设置当前进程的屏蔽信号集，并将旧的屏蔽信号集保存到 old_set 中
    // 用户发送了 SIGINT（通常是通过按下 Ctrl+C），信号也不会被递送到进程中。
    if(sigprocmask(SIG_BLOCK, &new_set, &old_set) == -1)
    { 
        perror("Sigprocmask Failed");
        exit(EXIT_FAILURE);
    }

    printf("SIGINT is blocked. Try pressing Ctrl+C. You will see that the signal is pending.\n");
    sleep(10); // 给出足够时间让用户有机会发送 SIGINT 信号，但是信号被阻塞了

    // 获取当前进程的未决信号集，即当前进程收到但是由于被阻塞等原因未被处理的信号的信号集
    if(sigpending(&pending_set) == -1)
    {
        perror("Sigpending Failed");
        exit(EXIT_FAILURE);
    }

    // 判断 SIGINT 信号是否在未决信号集中，若在返回1，否则返回0
    if(sigismember(&pending_set, SIGINT) == 1)
    {
        printf("SIGINT is pending\n");
    }
    else
    {
        printf("SIGINT is not pending\n");
    }

    // 恢复到旧的信号屏蔽字，解除对 SIGINT 信号的阻塞
    if(sigprocmask(SIG_SETMASK, &old_set, NULL) == -1)
    {
        perror("Sigprocmask Failed");
        exit(EXIT_FAILURE);
    }

    // 如果在阻塞期间有 SIGINT 信号发送到进程，此信号将在解除阻塞后立即被处理。
    printf("SIGINT is unblocked.\n");

    // 程序再次睡眠一秒钟，以确保信号处理函数有机会执行。
    sleep(1); 

    return 0;
}
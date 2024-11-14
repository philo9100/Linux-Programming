#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

void sig_alrm(int signo)
{
    // 什么都不做，只是为了唤醒pause
}

unsigned int my_sleep(unsigned int sec)
{
    struct sigaction new_act, old_act;
    new_act.sa_handler = sig_alrm;
    sigemptyset(&new_act.sa_mask);
    new_act.sa_flags = 0;
    sigaction(SIGALRM, &new_act, &old_act); // 注册信号处理函数，捕获SIGALRM信号.执行信号处理函数，如果没有捕获SIGALRM信号将执行默认行为，那么进程将被终止。
    alarm(sec); // 设置定时器，sec秒后超时，向当前进程发送SIGALRM信号
    sigaction(SIGALRM, &old_act, NULL); // old_act中没有绑定SIGALRM的信号处理函数，即恢复SIGALRM信号的默认行为
    pause(); // 暂停进程，直到接收到一个信号
    return alarm(0); // 取消定时器，返回定时器剩余的时间
}

int main()
{
    while(1)
    {
        my_sleep(2);
        printf("two seconds passed\n");
    }
    return 0;
}
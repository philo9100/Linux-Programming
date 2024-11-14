// 该程序实现周期定时
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>

unsigned int my_cycle_alarm(unsigned int sec, unsigned int interval_sec)
{
    struct itimerval new_value, old_value;
    int ret;
    new_value.it_interval.tv_sec = interval_sec;    // 定时器间隔秒    
    new_value.it_interval.tv_usec = 0;  // 微秒 
    new_value.it_value.tv_sec = sec;    // 定时器秒
    new_value.it_value.tv_usec = 0; // 微秒
    ret = setitimer(ITIMER_REAL, &new_value, &old_value);
    if(ret == -1)
    {
        perror("Setitimer Failed");
        exit(EXIT_FAILURE);
    }
    return old_value.it_value.tv_sec;
}

// 该程序实现单次定时
void alarm_handler(int signo)
{
    printf("Caught the SIGALARM signal\n");
}

int main()
{
    int ret;
    struct sigaction sig_act;
    sig_act.sa_handler = alarm_handler;
    sigemptyset(&sig_act.sa_mask);
    sig_act.sa_flags = 0;

    if(ret = sigaction(SIGALRM, &sig_act, NULL) == -1)
    {
        perror("Sigaction Failed");
        exit(EXIT_FAILURE);
    }

    while(1)
    {
        my_cycle_alarm(3, 1); // 设置定时器，3秒后超时，之后每2秒超时一次
        
        pause();    // 暂停进程，直到接收到一个信号
    }

}
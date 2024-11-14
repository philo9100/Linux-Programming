#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>


// 信号处理函数
void signal_handler(int signo)
{
    printf("Caught the SIGINT signal\n");
    sleep(5);   // 模拟信号处理函数执行
}

int main()
{
    struct sigaction act, old_act;
    act.sa_handler = signal_handler;    // 用于指定信号处理函数的地址，可以是一个函数指针，也可以是SIG_IGN或SIG_DFL。
    sigemptyset(&act.sa_mask);  // 指定在信号处理函数执行期间需要阻塞的信号集合。
    sigaddset(&act.sa_mask, SIGINT);    // 将 SIGINT 信号添加到信号集中
    act.sa_flags = SA_RESETHAND;   // 执行完信号处理函数后，将信号的处理程序重置为SIG_DFL。
    /*
    第一次按下ctrl+c，执行信号处理函数，然后将信号处理函数重置为默认行为
    第二次按下ctrl+c，执行默认行为，即终止进程
    */
    sigaction(SIGINT, &act, &old_act);  // 注册信号处理函数

    while(1)
    {
        printf("Try pressing Ctrl+C\n");
        printf("This is parent process, pid = %d\n", getpid());
        sleep(1);
    }

}
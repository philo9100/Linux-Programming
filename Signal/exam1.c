#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <string.h>


void printset(sigset_t *sigset){
    for (int i = 1; i < NSIG; i++) {
        if (sigismember(sigset, i) == 1) {
            putchar('1');
        }
        else {
            putchar('0');
        }
    }    
    printf("\n");
}

int main(int argc, char **argv)
{
    sigset_t sig_set;
    int ret;
    ret = sigemptyset(&sig_set);    // 将信号集清空
    if(ret == -1)
    {
        fprintf(stderr, "sigemptyset: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    ret = sigaddset(&sig_set, SIGSTOP);
    if(ret == -1)
    {
        fprintf(stderr, "sigaddset: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    ret = sigismember(&sig_set, SIGSTOP);
    if(ret == -1)
    {
        fprintf(stderr, "sigismember: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    printf("SIGSTOP is %sin set\n", ret == 1? "":"not ");

    ret = sigismember(&sig_set, SIGINT);
    if(ret == -1)
    {
        fprintf(stderr, "sigismember: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    printf("SIGINT is %s in set\n", ret == 1? "":"not");

    /* 
    Prototype for the glibc wrapper function 
    int sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
    */
    sigset_t oldset, pendingset;
    // 将sig_set作为进程的信号屏蔽集
    sigprocmask(SIG_BLOCK, &sig_set, &oldset);

    // 当前进程在添加信号屏蔽集之前的信号屏蔽集是oldset
    printf("oldset val is %ln\n",oldset.__val); 
    printset(&oldset);
    printset(&sig_set);
    
    // 发送SIGSTOP信号查看屏蔽的信号SIGSTOP是否在未决信号集中
    while (1) {
        sigpending(&pendingset);
        printset(&pendingset);
        sleep(1);
    }
    
    sleep(3);

    // 将进程最初的信号集重新作为屏蔽信号集，oldset应该是全零，即不屏蔽信号
    sigprocmask(SIG_BLOCK, &oldset, NULL);

    // 发送SIGSTOP信号查看屏蔽的信号SIGSTOP是否在未决信号集中
    while (1) {
        sigpending(&pendingset);
        printset(&pendingset);
        sleep(1);
    }
    
    return 0;

}



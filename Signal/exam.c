#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/time.h>

void sig_cont(int signo){
    printf("caught a signal of SIGCONT");
}

int main(){
    pid_t pid;
    pid = fork();

    if (pid == -1) {
        fprintf(stderr, "failed to fork:%s", strerror(errno));
        exit(EXIT_FAILURE);
    }   
    else if (pid == 0) {
        signal(SIGCONT, sig_cont); // 先注册信号处理函数
        printf("This is child process, pid = %d, I am going to stop myself\n", getpid());
        // if(raise(SIGSTOP) != 0) // 子进程给自己发送一个sigstop信号
        // {
        //     exit(EXIT_FAILURE);
        // }
        pause(); // 进程挂起等待信号递达。要么执行信号处理函数，要么结束进程

        while (1) {
            printf("This is child process, pid = %d, I am alive!\n", getpid());
            sleep(1);
        }
    }
    else if (pid > 0){
        sleep(5); // 这里先让父进程睡5秒
        kill(pid, SIGCONT);
        sleep(2);
        kill(pid, SIGKILL);
        waitpid(pid, NULL, 0);
        // 测试是否会执行到这里   没有执行到这里
        printf("This is parent process, pid = %d\nChild process has already been killed\n", getpid());
    }

    return 0;
}
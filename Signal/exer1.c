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
        sleep(3); // 子进程正在执行sleep函数（即处于睡眠状态）时，父进程发送了SIGSTOP信号，那么子进程将被停止。如果父进程随后发送了SIGCONT信号，那么子进程将从停止状态恢复，继续执行之前的工作。如果子进程在睡眠状态时接收到SIGKILL信号，那么子进程将被立即终止。
        printf("This is child process, pid = %d, ppid = %d\n", getpid(), getppid());
        sleep(3);
        kill(getppid(), SIGKILL); // 在向父进程发送SIGKILL信号后，父进程将被立即终止，子进程将被init进程接管，子进程的ppid将被设置为1。
    }
    else if(pid > 0) // 父进程
    {
        kill(pid, SIGSTOP); // 向子进程发送SIGSTOP信号,使子进程暂停运行子进程，即使处于睡眠状态，也能处理父进程发送的信号。当一个进程接收到信号时，无论它当前处于何种状态（运行、睡眠、停止等），操作系统都会将该信号传递给进程。如果进程已经为该信号注册了处理函数，那么处理函数将被调用。如果没有注册处理函数，那么将执行该信号的默认行为。
        printf("This is parent process, pid = %d\nChild process has already been stopped\n", getpid());

        kill(pid, SIGCONT); // 向子进程发送SIGCONT信号,使子进程继续运行
        printf("Child process has already been continued\n");
        // 等待子进程结束，之后处理子进程发送的SIGKILL信号
        printf("Child process has already finished\n I will be killed\n");
        wait(NULL); // 子进程可以发送SIGKILL信号尝试杀死父进程。然而，如果父进程在接收到SIGKILL信号之前已经调用了wait函数并阻塞在那里等待子进程结束，那么父进程将不会被立即杀死。这是因为wait函数会导致父进程进入阻塞状态，直到一个子进程结束。当子进程结束后，wait函数将返回，父进程将从阻塞状态中恢复，然后处理未决的信号，包括子进程发送的SIGKILL信号。这时，父进程将被SIGKILL信号杀死。需要注意的是，SIGKILL信号是不能被忽略或捕获的，所以一旦父进程从阻塞状态中恢复并开始处理未决的信号，它将被立即杀死。
    }

    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main()
{
    pid_t pid; // 声明进程标识符
    pid = fork(); // 创建子进程

    if (pid == -1) // 进程创建失败
    {
        perror("Fork Faild");
        exit(EXIT_FAILURE);
    }
    else if(pid > 0) // 父进程
    {
        /*
        在C语言中，wait()函数用于使父进程暂停执行，直到它的一个子进程结束为止。wait()函数的参数是一个指向int类型的指针，用于存储子进程的退出状态。如果你将NULL作为参数传递给wait()函数，那么意味着你不关心子进程的退出状态。你只是想让父进程等待，直到有一个子进程结束。在这种情况下，子进程的退出状态将被丢弃。这是一个常见的用法，因为在许多情况下，父进程并不真正关心子进程的退出状态，它只是需要知道子进程何时结束。*/
        wait(NULL); // 等待子进程结束，不考虑子进程的退出状态
        printf("This is parent process, pid = %d\n", getpid());
        printf("child process has finished\n");
    }
    else if(pid == 0) // 子进程
    {
        char* arguePath = "/home/philo/professional/CSCourse/LinuxPractice/Process/exer1";
        printf("This is child process, pid = %d\n", getpid());
        // execl("/bin/ls", "ls", "-l", arguePath, NULL); // 执行ls -l命令
        /*
        在执行exec()被调用并成功，它就不会返回到原来的程序中，也就是说下面的程序不会再执行，这也是exec函数族存在的意义
        若要执行下面的程序，需要在exec函数族调用之前创建子进程或者注释掉上面的exec函数族调用
        */
        sleep(5);
        
        char *arguePath2[] = {"ls", "-l", arguePath, NULL};
        execv("/bin/ls", arguePath2); // 执行ls -l命令

        perror("Exec Faild"); // 如果执行失败，输出错误信息
        printf("child process pid = %d has finished\n", getpid());
    }

    return 0;
}

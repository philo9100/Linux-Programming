#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>

#define BUFFER_SIZE 1024

int main()
{
    int fd[2]; // 定义文件描述符数组，fd[0]用于读取管道，fd[1]用于写入管道
    int ret;    
    pid_t pid; // 定义进程ID

    if (ret = pipe(fd) == -1) // 创建管道
    {   // 管道创建失败
        perror("pipe error");
        exit(EXIT_FAILURE);
    }

    pid = fork(); // 创建子进程
    // 在Linux中，经由fork()函数创建的子进程会继承父进程的虚拟地址空间。这意味着子进程将会拥有父进程的所有内存映射和数据，包括代码、数据、堆栈等。遵循 读时共享，写时复制原则父子进程共享文件描述符，因此父进程创建的管道子进程也可以使用。
    // 在Linux中，当父进程或子进程向管道写入数据时，并不会发生写时复制。写时复制是针对内存页的复制，在涉及文件描述符和管道时，并不适用于相同的方式。当进程向管道写入数据时，数据会被传输到管道的缓冲区中，而不会立即进行复制。这是因为管道是基于内核缓冲区的，内核会自动处理数据的复制和传输。因此，写入管道并不会触发写时复制机制。因此，当父进程执行write操作时，并不会直接触发写时复制。相反，当子进程尝试在共享的内存页上执行write操作时，将会触发写时复制机制，以便确保父子进程之间的数据隔离。

    if(pid == -1)
    {
        perror("fork error");
        exit(EXIT_FAILURE);
    }
    else if(pid == 0)
    {
        // 子进程--读管道
        close(fd[1]);   // 关闭子进程的写端
        char buf[BUFFER_SIZE];  // 定义缓冲区
        while(read(fd[0], buf, strlen(buf)) > 0) // 从管道中读取数据
        {
            write(STDOUT_FILENO, buf, strlen(buf)); // 将数据写入标准输出
        }
        write(STDOUT_FILENO, "\n", 1); // 写入换行符
        close(fd[0]);   // 关闭子进程的读端
        exit(EXIT_SUCCESS);
    }
    else
    {
        // 父进程--写管道
        close(fd[0]);   // 关闭父进程的读端
        char *str = "hello world";
        write(fd[1], str, strlen(str)); // 将数据写入管道
        close(fd[1]);  // 关闭父进程的写端
        wait(NULL); // 等待子进程结束
        exit(EXIT_SUCCESS);
    }
    return 0;
}
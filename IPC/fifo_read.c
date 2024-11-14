// fifo_read.c
// gcc fifo_read.c -o fifo_read -pthread
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <semaphore.h>

// 在读取程序中定义一个信号量
sem_t *sem;
#define BUFFER_SIZE 1024

int main(int argc, char** argv)
{
    if(argc < 2)
    {
        printf("Usage: %s <fifo>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int ret;
    ret = access(argv[1], F_OK);  // 检查命名管道是否存在
    if(ret == -1)   // 若命名管道不存在，就创建命名管道
    {
        int r = mkfifo(argv[1], 0664);  // 创建命名管道
        if(r == -1)
        {
            perror("mkfifo error");
            exit(EXIT_FAILURE);
        }
        else
        {
            printf("create fifo %s success\n", argv[1]);
        }
    }

    // 在读取程序中打开信号量
    sem = sem_open("my_sem", 0); //打开之前创建的信号量
    if(sem == SEM_FAILED)
    {
        perror("sem_open error");
        exit(EXIT_FAILURE);
    }

    int fd; // 定义文件描述符
    fd = open(argv[1], O_RDONLY);    // 打开命名管道
    if(fd == -1)
    {
        perror("open error");
        exit(EXIT_FAILURE);
    }

    while(1)
    {
        char buf[BUFFER_SIZE] = {0};  // 定义缓冲区
        sem_wait(sem);  // 等待写入程序发送信号
        read(fd, buf, sizeof(buf)); // 从命名管道中读取数据
        write(STDOUT_FILENO, buf, strlen(buf)); // 将数据写入标准输出
        fflush(stdout); // 刷新标准输出
        memset(buf, 0, sizeof(buf));    // 清空缓冲区
    }

    close(fd);  // 关闭文件描述符
    return 0;
}
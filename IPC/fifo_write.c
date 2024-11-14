// fifo_write.c
// gcc fifo_write.c -o fifo_write -pthread
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <semaphore.h>

// 在写入程序中定义一个信号量
sem_t *sem;

#define BUFFER_SIZE 1024

int main(int argc, char **argv)
{
    if(argc < 2)    // 检查命令行参数，判断是否传入文件名
    {
        printf("Usage: %s <fifo>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    // 在写入程序中打开信号量
    sem = sem_open("my_sem", O_CREAT, 0664, 0); // 信号量初始值为0

    int ret;
    ret = access(argv[1], F_OK);  // 检查命名管道是否存在
    /*
    在Linux中，access 函数用于检查调用进程对指定文件的访问权限。它是由 <unistd.h> 头文件提供的 POSIX 函数。access 函数原型如下：

    #include <unistd.h>
    int access(const char *pathname, int mode);
    这个函数的参数包括：

    pathname：要检查的文件或目录的路径名。
    mode：一个位掩码，指定要检查的访问权限。可以使用以下几个标志：
    F_OK：检查文件是否存在。
    R_OK：检查是否有读取权限。
    W_OK：检查是否有写入权限。
    X_OK：检查是否有执行权限。
    这些标志可以通过逻辑 OR 运算符组合使用，例如，检查读写权限可以写成 R_OK | W_OK。

    返回值：

    如果权限检查成功，返回 0。
    如果权限检查失败，返回 -1，同时设置 errno 以指示错误类型。
    */
    if(ret == -1)   // 若命名管道不存才，就创建命名管道
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
    
    int fd; // 定义文件描述符
    fd = open(argv[1], O_WRONLY);    // 打开命名管道

    if(fd == -1)
    {
        perror("open error");
        exit(EXIT_FAILURE);
    }

    while(1)
    {
        char buf[BUFFER_SIZE];  // 定义缓冲区
        fgets(buf, BUFFER_SIZE, stdin); // 从标准输入读取数据
        // scanf("%s", buf);
        write(fd, buf, strlen(buf));    // 将数据写入命名管道
        sem_post(sem);  // 发送信号量通知读取程序开始读取数据
        memset(buf, 0, sizeof(buf));    // 清空缓冲区        
    }

    close(fd);  // 关闭文件描述符
    return 0;
}
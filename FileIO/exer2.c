#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

int main() {
    // 打开一个文件
    int fd = open("dup2.txt", O_RDWR | O_CREAT, 0664);
    if (fd == -1) 
    {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }
    
    // 将打开的文件描述符复制到新的文件描述符，newfd与fd指向同一个文件表项
    int newfd1 = dup(fd);
    if (newfd1 == -1) 
    {
        perror("Failed to dupliate file descriptor");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // 关闭原始的文件描述符
    close(fd);

    // 
    
    if (dup2(newfd1, STDOUT_FILENO) == -1) 
    {
        perror("Failed to dupliate file descriptor");
        close(newfd1);
        exit(EXIT_FAILURE);
    }

   // 现在，printf将输出到文件中
    printf("This will be written to the file\n");

    close(newfd1);

    return 0;
}
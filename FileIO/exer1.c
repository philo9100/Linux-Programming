#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

int main()
{
    // 文件描述符
    int fd;

    // 指明文件路径
    char *filePath = "/home/philo/professional/CSCourse/LinuxPractice/FileIO/test.txt";

    // 打开文件，使用open()函数
    // open()函数原型：int open(const char *pathname, int flags, mode_t mode);
    // open函数返回文件描述符，如果出错返回-1，pathname为文件路径，flags为打开文件的方式，mode为文件权限
    fd = open(filePath, O_RDWR | O_CREAT | O_TRUNC, 0777);

    // 判断文件是否打开成功
    if (fd == -1)
    {
        perror("file open error\n");
        exit(EXIT_FAILURE);
    } else
    {
        printf("file open success\n");
    }

    // 写入文件
    // char *str = "hello world\n";
    // 计算字符串长度
    // int len = strlen(str);
    
    // 定义字符缓冲区
    char buffer[1024];
    // 从键盘输入字符串
    scanf("%s", buffer);
    // 计算字符串长度，字节数
    int len = strlen(buffer);
    // 写入文件，使用write()函数
    // write()函数原型：ssize_t write(int fd, const void *buf, size_t count);
    // write()函数返回写入的字节数，如果出错返回-1，fd为文件描述符，buf为写入的内容，count为写入的字节数
    int size = write(fd, buffer, len);

    // 判断写入是否成功
    if (size == -1)
    {
        perror("file write error\n");
        close(fd);
        exit(EXIT_FAILURE);
    }else
    {
        printf("file write success\n");
    }

    // 关闭文件，使用close()函数
    // close()函数原型：int close(int fd);
    // close()函数返回0，如果出错返回-1，fd为文件描述符
    int ret = close(fd);

    // 判断关闭是否成功
    if (ret == -1)
    {
        perror("file close error\n");
        exit(EXIT_FAILURE);
    }else
    {
        printf("file close success\n");
    }

    printf("**************************************************************\n");
    // 再次打开文件，使用open()函数
    fd = open(filePath, O_RDONLY);
    // 判断文件是否打开成功
    if(fd == -1)
    {
        perror("file open error\n");
        exit(EXIT_FAILURE);
    }else
    {
        printf("file open again success\n");
    }

    // 声明文件大小
    off_t f_size = 0;

    // 获取文件偏移量，使用lseek()函数
    // lseek()函数原型：off_t lseek(int fd, off_t offset, int whence);
    // lseek()函数返回文件偏移量，如果出错返回-1，fd为文件描述符，offset为偏移量，whence为偏移量的起始位置
    f_size = lseek(fd, 0, SEEK_END);

    lseek(fd, 0, SEEK_SET);
    // 循环读取文件
    int size_read;
    while(lseek(fd, 0, SEEK_CUR) != f_size)
    {
        // 读取文件，使用read()函数
        // read()函数原型：ssize_t read(int fd, void *buf, size_t count);
        // read()函数返回读取的字节数，如果出错返回-1，fd为文件描述符，buf为读取的内容，count为读取的字节数
        size_read = read(fd, buffer, 1024);
        // 判断读取是否成功
        if (size_read == -1)
        {
            perror("file read error\n");
            close(fd);
            exit(EXIT_FAILURE);
        }
        printf("%s\n", buffer);
    }

    // 关闭文件，使用close()函数
    ret = close(fd);
    // 判断关闭是否成功
    if (ret == -1)
    {
        perror("file close error\n");
        exit(EXIT_FAILURE);
    }else
    {
        printf("file close success\n");
    }

    return 0;
}

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define BUFFER_SIZE 1024

int main()
{
    FILE *r_fp, *w_fp;   // 定义文件指针
    char buf[BUFFER_SIZE]; // 定义缓冲区

    r_fp = popen("ls -l", "r");    // 执行ls -l命令，创建一个管道，返回一个文件指针
    w_fp = popen("wc -l", "w");    // 执行wc -l命令，创建一个管道，返回一个文件指针

    if(r_fp == NULL || w_fp == NULL)
    {
        perror("popen error");
        exit(EXIT_FAILURE);
    }

    // 从管道中读取数据
    while(fgets(buf, sizeof(buf), r_fp) != NULL)
    {
        // 将数据写入管道
        fputs(buf, w_fp);
    }
    
    // 关闭文件指针
    pclose(r_fp);
    pclose(w_fp);
    return 0;
}
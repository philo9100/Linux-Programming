// tcpclient.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 6666  
#define MAXLINE 1024

int main(int argc, char **argv)
{
    struct sockaddr_in servaddr;    // 服务器端地址信息
    char buf[MAXLINE];              // 读取数据的缓冲区
    int sockfd, n;
    char *str;  // 
    if(argc != 2)
    {
        fputs("usage: ./client message\n", stderr);
        exit(EXIT_FAILURE);
    }
    str = argv[1];  // 获取命令行参数，argv[1]为要发送的数据是char *类型

    // 创建客户端socket，使用IPv4协议，面向连接通信，TCP协议
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    // 初始化服务器端口地址信息为空
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;    // IPv4协议
    // 设置服务器端IP地址
    /*
    inet_pton()函数用于将IP地址的字符串形式转换为网络字节序的二进制形式。
    这个函数的名字"pton"代表"presentation to network"
    意思是将IP地址从人类可读的形式（presentation format）转换为网络传输的形式（network byte order format）。
    */
    inet_pton(AF_INET, SERVER_IP, &servaddr.sin_addr);
    servaddr.sin_port = htons(SERVER_PORT);    // 设置服务器端口号

    // 向服务器端发送连接请求
    connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

     // 从标准输入读取数据发送到服务器端，然后接收服务器端返回的数据
    send(sockfd, str, strlen(str), 0);
    n = recv(sockfd, buf, MAXLINE, 0);
    // 打印接收到的数据
    printf("Response from server:");
    write(STDOUT_FILENO, buf, n);
    printf("\n");

    fputs("Input message to send:(Q to quit)\n", stdout);
    while(1)
    {
        // 
        fgets(buf, MAXLINE, stdin);

        // 如果输入Q，则退出
        if(!strcmp(buf, "q\n") || !strcmp(buf, "Q\n"))
            break;
        
        // 将输入的数据发送给服务器端
        send(sockfd, buf, strlen(buf), 0);

        n = recv(sockfd, buf, MAXLINE, 0);

        // 打印接收到的数据
        printf("Response from server:\n");
        write(STDOUT_FILENO, buf, n);
    }
   
    // 关闭连接
    close(sockfd);

    return 0;
}
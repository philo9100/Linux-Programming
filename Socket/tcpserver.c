//tcpserver.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define SERVER_PORT 6666    // 定义服务器端口号
#define MAXLINE 1024        // 定义缓冲区大小

int main()
{
    struct sockaddr_in server_addr, client_addr;    // 声明服务器和客户端的地址信息的结构体
    socklen_t client_addr_len;    // 声明客户端地址信息结构体的大小
    int listenfd, connfd;         // 声明服务器的监听socket和服务器与客户端的连接socket
    char buf[MAXLINE];            // 声明读取数据的缓冲区
    /*
    INET_ADDRSTRLEN是一个宏，定义在<netinet/in.h>头文件中。
    它表示IPv4地址字符串的最大长度。IPv4地址的字符串形式通常是“xxx.xxx.xxx.xxx”，最多有15个字符，加上结束符'\0'
    所以INET_ADDRSTRLEN的值通常被定义为16。这个宏常常被用于定义存储IPv4地址字符串的字符数组的大小。   
    */
    char str[INET_ADDRSTRLEN];    // 保存客户端地址信息的字符串
    int i, n;

    // 创建服务器端socket，使用IPv4协议，面向连接通信，TCP协议，返回监听socket文件描述符
    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    // 设置服务器端地址信息
    bzero(&server_addr, sizeof(server_addr));    // 初始化服务器端地址信息结构体对应的内存空间为0
    server_addr.sin_family = AF_INET;            // IPv4协议
    /*
    INADDR_ANY是一个特殊的值，表示服务器socket可以接受任何网络接口上的连接请求。
    htonl()用于将主机字节序（小端存储）转换为网络字节序（大端存储）。
    */
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
    server_addr.sin_port = htons(SERVER_PORT);    // 设置端口号

    // 将服务器端监听socket与服务器端地址信息绑定
    bind(listenfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    // 将监听socket设置最大监听队列长度为128
    listen(listenfd, 128);

    printf("waiting for client connect...\n");

    while(1)
    {
        client_addr_len = sizeof(client_addr); // 获取客户端地址信息的大小

        // 从已完成连接队列中取出一个连接请求，如果已完成连接队列为空，则阻塞等待
        connfd = accept(listenfd, (struct sockaddr *)&client_addr, &client_addr_len);

        // 打印客户端地址信息
        printf("client IP: %s\tport: %d\n", inet_ntop(AF_INET, &client_addr.sin_addr, str, sizeof(str)), ntohs(client_addr.sin_port));

        // 读取客户端发送的数据，存放在缓冲区buf中，返回读取到的字节数
        n = recv(connfd, buf, MAXLINE, 0);

        buf[n] = '\0';  // 在字符串末尾添加结束符'\0'
        printf("received from client: %s\n", buf);
        
        if(!strcmp(buf, "Q") || !strcmp(buf, "q"))
            break;

        // 打印读取到的数据，并将数据转换为大写发送回客户端
        for(i = 0; i < n; i++)
        {
            // 将小写字母转换为大写字母重新发存入缓冲区
            buf[i] = toupper(buf[i]);
        }
        send(connfd, buf, n, 0);

        // 关闭连接socket
        close(connfd);
    }
    return 0;
}
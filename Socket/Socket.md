# Socket编程

## Socket编程接口

在Linux中，Socket是一种用于进程间通信的机制，特别是在不同主机之间的进程。Socket提供了一种方式，使得任何网络上的两个进程都可以进行数据交换。

Socket 本质上是一种编程接口（API）为了提供一个标准化的方式来进行网络通信。在网络通信的早期，没有一种统一的方法来处理不同类型的网络通信。Socket 的引入提供了一种通用的、一致的编程接口，使得开发者在写程序时不需要关心底层的网络通信细节，可以跨多种网络协议（如 TCP、UDP 等）和物理层进行数据传输。

> 在网络通信中，"端点"通常指的是网络连接的一端，它通常由一个IP地址和一个端口号组成。

Linux中常用的socket网络编程接口socket()、bind()、listen()、accept()、connect()、send()、recv()、close()等函数。

> `socket()`
>
> socket()函数用于创建一个socket，返回一个socket描述符，该描述符唯一标识一个socket。该函数存在于头文件`<sys/socket.h>`中，其原型为：
>
> ```c
> int socket(int domain, int type, int protocol);
> ```
>
> 参数说明：
>
> - `domain`：表示使用的协议族，常用的协议族有`AF_INET`、`AF_INET6`、`AF_LOCAL`（或称`AF_UNIX`，Unix域socket）、`AF_ROUTE`等等，协议族决定了socket的地址类型，在通信中必须采用对应的地址，如**AF_INET决定了要用ipv4地址（32位的）与端口号（16位的）的组合，可以实现与远程端点通信连接**、**AF_UNIX决定了要用一个绝对路径名作为地址，可以实现本地系统进程间的通信连接**；
> - `type`：表示socket类型，常用的socket类型有`SOCK_STREAM`、`SOCK_DGRAM`、`SOCK_RAW`等等，`SOCK_STREAM`（流式socket，即TCP），提供**按顺序、可靠、双向、面向连接且基于字节流的通信**，**传输效率低**，传输单位是字节；  `SOCK_DGRAM`（数据报式socket，即UDP）提供**定长、不可靠、无连接且基于数据报的通信**，**数据可能丢失或重复，可能不按序到达**，**传输效率高**，传输单位是数据报；  
> `SOCK_RAW`（raw socket，即原始套接字）提供**对网络层及以下的直接访问且不会被操作系统的 TCP/IP 协议栈所处理**，程序可以自行构造和发送原始的网络数据包，也可以接收网络数据包并进行解析。一般用于网络层的测试或者某些特殊应用；
> - `protocol`：表示协议，常用的协议有`IPPROTO_TCP`、`IPPROTO_UDP`、`IPPROTO_SCTP`、`IPPROTO_TIPC`等等，协议决定了要使用哪个协议来进行通信，如IPPROTO_TCP决定了使用TCP协议进行通信，IPPROTO_UDP决定了使用UDP协议进行通信，IPPROTO_SCTP决定了使用SCTP协议进行通信，IPPROTO_TIPC决定了使用TIPC协议进行通信。**如果`protocol`为0，表示使用`domain`和`type`参数所确定的默认协议**。
>
> socket()函数成功时返回一个socket描述符，失败时返回-1并设置errno。

在UDP中，"定长"通常指的是数据报的大小是固定的。每个UDP数据报的大小限制通常是64KB（包括数据和头部信息）。

> `bind()`
> bind()函数用于将一个socket与一个特定的IP地址和端口绑定。该函数存在于头文件`<sys/socket.h>`中，其原型为：
>
> ```c
> int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
> ```
>
> 参数说明：
>
> - `sockfd`：表示要绑定的socket描述符；
> - `addr`：表示要绑定的IP地址和端口号，是一个`sockaddr`结构体指针，该结构体中包含了要绑定的IP地址和端口号，其定义如下：
>
> ```c
> struct sockaddr { // 结构体大小为16字节
>     sa_family_t sa_family; // 地址族，如AF_INET、AF_INET6、AF_LOCAL等
>     char sa_data[14]; // 地址信息，包含IP地址和端口号，大小为14字节，其中前两个字节表示端口号，后12个字节表示IP地址。
> };
> ```
>
> bind 函数声明为接受通用的 sockaddr 结构体指针，但在实际调用时需要传入具体的结构体类型，为了让bind()函数可以接受不同类型的地址结构体。例如，对于IPv4和IPv6，我们分别使用`struct sockaddr_in`和`struct sockaddr_in6`，这两个结构体的大小是不同的，但是只是传入指针变量。
>
> ```c
> struct sockaddr_in {
>     sa_family_t sin_family; // 地址族，AF_INET
>     in_port_t sin_port; // 端口号
>     struct in_addr sin_addr; // IP地址
> };
> struct in_addr {  // 这个结构体大小为4字节
>     uint32_t s_addr; // IP地址，32位无符号整数
> };
>
> stuuct sockaddr_in6 {
>     sa_family_t sin6_family; // 地址族，AF_INET6
>     in_port_t sin6_port; // 端口号
>     uint32_t sin6_flowinfo; // 流信息
>     struct in6_addr sin6_addr; // IP地址
>     uint32_t sin6_scope_id; // 作用域ID
> };
>```
>
> - `addrlen`：表示`addr`结构体的大小，单位是字节。
> bind()函数成功时返回0，失败时返回-1并设置errno。

bind()函数的第三个参数addrlen是必要的，它告诉函数你传入的地址结构体的实际大小，以便函数可以正确地处理这个结构体。

> `listen()`
> listen()函数用于将一个未连接的socket转换成一个被动监听状态的socket，指示内核应该接受指向该socket的连接请求。该函数存在于头文件`<sys/socket.h>`中，其原型为：
>
> ```c
> int listen(int sockfd, int backlog);
> ```
>
> 参数说明：
>
> - `sockfd`：表示要监听的socket描述符；
> - `backlog`：指定了队列中最多可以有多少个等待监听端点接受的连接请求，上限由 /proc/sys/net/core/somaxconn 内核参数定义（在大多数系统上默认值为 128）
>
> listen()函数成功时返回0，失败时返回-1并设置errno。

当一个请求端点尝试与使用 listen() 函数的监听端点建立连接时，一个新的连接请求会被放入到队列中。如果队列已满，后来的客户端请求可能会被拒绝。队列实际上分为两个子队列：

- 未完成连接队列（incomplete connection queue）：存放那些已经收到请求端点的 SYN（同步）报文，但是监听端点还没有完成三次握手过程的连接。
- 已完成连接队列（completed connection queue）：存放那些已经完成三次握手，等待服务器调用 accept() 函数的连接。

当客户端尝试与服务器建立 TCP 连接时，连接的建立过程经历了三次握手。在这个过程中，首先客户端发送一个 SYN（同步）报文到服务器，表示客户端请求建立连接。服务器收到 SYN 报文后，会响应一个 SYN-ACK 报文给客户端，表示同意建立连接。最后，客户端再发送一个 ACK 报文给服务器，表示确认连接建立。当客户端发送 SYN 报文后，服务器会将客户端的信息记录在未完成连接队列中，然后响应一个 SYN-ACK 给客户端。客户端发送的 SYN 报文是 TCP 连接建立过程的一部分。

> `accept()`
>
> accept()函数用于从已完成连接队列中取出一个连接请求，如果已完成连接队列为空，则阻塞等待。该函数存在于头文件`<sys/socket.h>`中，其原型为：
>
> ```c
> int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
> ```
>
> 参数说明：
>
> - `sockfd`：表示要监听的socket描述符；
> - `addr`：表示要绑定的IP地址和端口号，是一个`sockaddr`结构体指针，表示请求端点的地址信息，当该参数设置为NULL，表示不关心请求端点的地址；
> - `addrlen`：表示`addr`结构体的大小，单位是字节。
>
> accept()函数成功时返回一个新的socket描述符，该描述符唯一标识一个socket，失败时返回-1并设置errno。

accept()函数返回一个新的socket描述符用来区分原始的监听socket和新建立的连接。

原始的监听socket（传递给accept()的那个）继续保持监听状态，可以接受更多的连接请求。而新的socket描述符则代表了一个具体的客户端连接，可以用来接收和发送数据。

这样做的好处是，服务器可以同时处理多个客户端连接。每当accept()函数接受一个新的连接请求，它就返回一个新的socket描述符，服务器就可以用这个描述符来与新的客户端进行通信，而不会影响到其他的客户端连接。

> `connect()`
>
> connect()函数用于建立与指定socket的连接。该函数存在于头文件`<sys/socket.h>`中，其原型为：
>
> ```c
> int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
> ```
>
> 参数说明：
>
> - `sockfd`：表示要请求建立连接的socket描述符；
> - `addr`：表示要请求的IP地址和端口号，是一个`sockaddr`结构体指针，表示请求端点的地址信息；
> - `addrlen`：表示`addr`结构体的大小，单位是字节。
>
> connect()函数成功时返回0，失败时返回-1并设置errno。

当客户端想要与服务器建立连接时，需要知道服务器的IP地址和端口号

> `send()`
>
> send()函数用于向指定socket发送数据。该函数存在于头文件`<sys/socket.h>`中，其原型为：
>
> ```c
> ssize_t send(int sockfd, const void *buf, size_t len, int flags);
> ```
>
> 参数说明：
>
> - `sockfd`：表示要发送数据的socket描述符；
> - `buf`：表示要发送的数据缓冲区的指针；
> - `len`：表示要发送的数据的大小，单位是字节；
> - `flags`：表示发送数据的行为，常用的有`MSG_DONTWAIT`（非阻塞发送）、`MSG_OOB`（发送紧急数据）等等。
>
> send()函数成功时返回实际发送的数据大小，失败时返回-1并设置errno。

flags参数是用来控制发送操作的行为的。以下是常用的flags参数取值：

- MSG_DONTWAIT：如果设定了这个标志，send()函数将以非阻塞的方式工作，即使没有足够的内核缓冲区空间，也会立即返回-1，而不是阻塞直到缓冲区有空间为止。

- MSG_NOSIGNAL：如果对方关闭了连接，send()函数会向程序发送 SIGPIPE 信号，导致程序退出。如果设置了这个标志，send()函数会忽略这个信号，而是返回-1并设置errno为 EPIPE。

- MSG_OOB：发送带外数据（Out of Band data），这些数据和普通数据分开，在TCP中很少用到。

这些标志位可以组合使用，比如 flags = MSG_DONTWAIT | MSG_NOSIGNAL。当使用多个标志位时，通过按位或的操作将它们组合起来。

> `sendto()`
>
> sendto()函数用于向指定socket发送数据。该函数存在于头文件`<sys/socket.h>`中，其原型为：
>
> ```c
> ssize_t sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);
> ```
>
> 参数说明：
>
> - `dest_addr`：表示要发送的IP地址和端口号，是一个`sockaddr`结构体指针；
> - `addrlen`：表示`addr`结构体的大小，单位是字节。
> sendto()函数成功时返回实际发送的数据大小，失败时返回-1并设置errno。

sendto()函数与send()函数的区别在于，send()函数用于已经建立连接的socket（如SOCK_STREAM类型的TCP socket）。在调用send()函数发送数据时，不需要指定目标地址，因为目标地址已经在建立连接时确定了。
sendto()函数则常用于无连接的socket（如SOCK_DGRAM类型的UDP socket）。在调用sendto()函数发送数据时，需要指定目标地址，因为UDP是无连接的，每次发送数据都可能发送到不同的地址。

> `sendmsg()`
>
> sendmsg()函数用于向指定socket发送数据。该函数存在于头文件`<sys/socket.h>`中，其原型为：
>
> ```c
> ssize_t sendmsg(int sockfd, const struct msghdr *msg, int flags);
> ```
>
> 参数说明：
>
> - `msg`：表示要发送的数据缓冲区结构体指针；
>
> ```c
> struct msghdr {
>     void *msg_name; // 指向目标地址的指针
>     socklen_t msg_namelen; // 目标地址的大小
>     struct iovec *msg_iov; // 指向数据缓冲区的指针
>     size_t msg_iovlen; // 数据缓冲区的大小
>     void *msg_control; // 指向辅助数据的指针
>     socklen_t msg_controllen; // 辅助数据的大小
>     int msg_flags; // 标志位
> };

sendmsg()函数使用一个msghdr结构体来描述消息和目标地址，这使得它比send()和sendto()函数更复杂，但也更强大。

> `recv()`
>
> recv()函数用于从指定socket接收数据。该函数存在于头文件`<sys/socket.h>`中，其原型为：
>
> ```c
> ssize_t recv(int sockfd, void *buf, size_t len, int flags);
> ```
>
> 参数说明：
>
> - `sockfd`：表示要接收数据的socket描述符；
> - `buf`：表示接收数据的缓冲区的指针；
> - `len`：表示接收数据的大小，单位是字节；
>
> `flags`：表示接收数据的行为，常用的有`MSG_DONTWAIT`（非阻塞接收）、`MSG_OOB`（接收紧急数据）等等。
> recv()函数成功时返回实际接收的数据大小，失败时返回-1并设置errno。
>
> `recvfrom()`
>
> recvfrom()函数用于从指定socket接收数据。该函数存在于头文件`<sys/socket.h>`中，其原型为：
>
> ```c
> ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen);
> ```
>
> 参数说明：
>
> - `src_addr`：表示发送端点的地址信息，是一个`sockaddr`结构体指针；
> - `addrlen`：表示`addr`结构体的大小，单位是字节。
>
> recvfrom()函数成功时返回实际接收的数据大小，失败时返回-1并设置errno。
>
> `recvmsg()`
>
> recvmsg()函数用于从指定socket接收数据。该函数存在于头文件`<sys/socket.h>`中，其原型为：
>
> ```c
> ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags);
> ```
>
>
> 
> `close()`
> close()函数用于关闭一个socket。该函数存在于头文件`<unistd.h>`中，其原型为：
>
> ```c
> int close(int fd);
> ```
>
> 参数说明：
>
> - `fd`：表示要关闭的socket描述符；
> close()函数成功时返回0，失败时返回-1并设置errno。

其他socket函数：

- `getsockopt()`：获取socket选项
- `setsockopt()`：设置socket选项
- `shutdown()`：关闭socket的读写功能
- `getpeername()`：获取连接端点的地址信息
- `getsockname()`：获取本地端点的地址信息
- `gethostname()`：获取本地主机名
- `gethostbyname()`：获取主机名对应的IP地址
- `gethostbyaddr()`：获取IP地址对应的主机名
- `getaddrinfo()`：获取主机名对应的IP地址
- `getnameinfo()`：获取IP地址对应的主机名

### Socket编程实例

#### TCP Socket编程实例

```c
//tcpserver.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define SERVER_PORT 6666    // 服务器端口号
#define MAXLINE 1024    // 缓冲区大小

int main()
{
    struct sockaddr_in server_addr, client_addr;    // 服务器和客户端的地址信息
    socklen_t client_addr_len;    // 客户端地址信息的大小
    int listenfd, connfd;    // 监听socket和连接socket
    char buf[MAXLINE];    // 读取数据的缓冲区
    char str[INET_ADDRSTRLEN];    // 保存客户端地址信息的字符串
    int i, n;

    // 创建服务器端socket，使用IPv4协议，面向连接通信，TCP协议，返回监听socket文件描述符
    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    // 设置服务器端地址信息
    bzero(&server_addr, sizeof(server_addr));    // 初始化服务器端地址信息为空
    server_addr.sin_family = AF_INET;    // IPv4协议
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);    // INADDR_ANY是一个特殊的值，表示服务器socket可以接受任何网络接口上的连接请求。htonl()用于将主机字节序转换为网络字节序。
    server_addr.sin_port = htons(SERVER_PORT);    // 设置端口号

    // 将socket与服务器端地址信息绑定
    bind(listenfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    // 将监听socket设置最大监听队列长度为128
    listen(listenfd, 128);

    printf("waiting for client connect...\n");

    while(1)
    {
        client_addr_len = sizeof(client_addr); // 获取客户端地址信息的大小

        connfd = accept(listenfd, (struct sockaddr *)&client_addr, &client_addr_len);    // 从已完成连接队列中取出一个连接请求，如果已完成连接队列为空，则阻塞等待

        // 打印客户端地址信息
        printf("client IP: %s\tport: %d\n", inet_ntop(AF_INET, &client_addr.sin_addr, str, sizeof(str)), ntohs(client_addr.sin_port));

        // 读取客户端发送的数据
        n = recv(connfd, buf, MAXLINE, 0);

        // 打印读取到的数据，并将数据转换为大写发送回客户端
        for(i = 0; i < n; i++)
        {
            printf("%c", buf[i]);
            buf[i] = toupper(buf[i]);
        }
        send(connfd, buf, n, 0);

        // 关闭连接socket
        close(connfd);
    }
    return 0;
}
```

```c
// tcpclient.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define SERVER_PORT 6666  
#define MAXLINE 1024

int main(int argc, char **argv)
{
    struct sockaddr_in servaddr;    // 服务器端地址信息
    char buf[MAXLINE];    // 读取数据的缓冲区
    int sockfd, n;
    char *str;  // 
    if(argc != 2)
    {
        fputs("usage: ./client message\n", stderr);
        exit(EXIT_FAILURE);
    }
    str = argv[1];

    // 创建客户端socket，使用IPv4协议，面向连接通信，TCP协议
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    // 初始化服务器端口地址信息为空
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;    // IPv4协议
    // 设置服务器端IP地址
    inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);
    servaddr.sin_port = htons(SERVER_PORT);    // 设置服务器端口号

    // 向服务器端发送连接请求
    connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    // 从标准输入读取数据发送到服务器端，然后接收服务器端返回的数据
    send(sockfd, str, strlen(str), 0);

    n = recv(sockfd, buf, MAXLINE, 0);

    // 打印接收到的数据
    printf("Response from server:\n");
    write(STDOUT_FILENO, buf, n);

    // 关闭连接
    close(sockfd);

    return 0;
}

```




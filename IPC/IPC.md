# 进程间通信

```bash
man -k pipe # 查看管道相关的命令
man 7 pipe # 查看管道相关的系统调用pipes and FIFOs
man 2 pipe # 查看管道相关的系统调用pipe
man 7 fifo # 查看命名管道的用法
```

## 1. 管道

### 匿名管道

> 在程序中使用匿名管道，需要先创建一个管道。Linux系统中创建匿名管道的函数是`pipe()`，该函数存在于函数库`unistd.h`中，其原型如下：
>
> ```c
>  int pipe(int pipefd[2]);
> ```
>
> - `pipefd`：是一个整型数组，该数组有两个元素，`pipefd[0]`和`pipefd[1]`，当在程序中使用`pipe()`创建管道时，程序可以通过传参的方式获取两个文件描述符，分别交给需要通信的两个进程。
> - `返回值`：若创建管道成功，则返回`0`，若创建管道失败，则返回`-1`。
>
> 匿名管道只能在有亲缘关系的进程间使用，匿名管道利用`fork`机制建立，刚创建的管道其读写两端都连在同一个进程上，在使用`fork()`函数创建子进程后，父子进程共享文件描述符，因此父子进程共享相同的管道，且管道读写两端也连在子进程上，fd[0]用于读取管道，fd[1]用于写入管道。管道通信是半双工通信，即只能单向传输数据，因此父子进程要关掉不需要的文件描述符。

#### 匿名管道程序示例1

```c
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

    ret = pipe(fd); // 创建管道
    if (ret == -1)
    {   // 管道创建失败
        perror("pipe error");
        exit(EXIT_FAILURE);
    }
    /*
    此时，父进程拥有fd[0]和fd[1]，即管道的读写两端都连在同一个进程上，fd[0]用于读取管道，fd[1]用于写入管道。
    */

    pid = fork(); // 创建子进程
    // 在Linux中，经由fork()函数创建的子进程会拥有和父进程相同的虚拟地址空间。这意味着子进程将会拥有父进程的所有内存映射和数据，包括代码、数据、堆栈等。遵循 "读时共享，写时复制"原则，父子进程共享文件描述符，因此父进程创建的管道子进程也拥有该管道文件描述符。
    // 在Linux中，当父进程或子进程向管道写入数据时，并不会发生写时复制。写时复制是针对内存页的复制，在涉及文件描述符和管道时，并不适用于相同的方式。当进程向管道写入数据时，数据会被传输到管道的内核缓冲区中，而不会进行复制。这是因为管道是基于内核缓冲区的，内核会自动处理数据的复制和传输。因此，写入管道并不会触发写时复制机制。因此，当父进程执行write操作写入管道文件时，并不会直接触发写时复制。而当父子进程尝试在共享的物理内存页面上执行write操作时，将会触发写时复制机制，以便确保父子进程之间的数据隔离。

    if(pid == -1)
    {
        perror("fork error");
        exit(EXIT_FAILURE);
    }
    else if(pid == 0)   
    {
        // 子进程--读管道
        close(fd[1]);   // 关闭子进程的写端
        char buf[BUFFER_SIZE] = {0};  // 定义缓冲区
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
```

#### 匿名管道程序示例2

```c
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdlib.h>

int main()
{
    int pipefd[2];  // 定义管道文件描述符数组，fd[0]用于读取管道，fd[1]用于写入管道
    int ret;
    ret = pipe(pipefd);   // 创建管道

    if(ret == -1)
    {   // 管道创建失败
        perror("pipe error");
        exit(EXIT_FAILURE);
    }

    int i = 0;
    pid_t pid, wpid;    // 定义进程描述符

    // 创建两个子进程
    // do
    // {
    //     pid = fork();   
    // } while (pid > 0 && i++ < 2);

    for(i; i < 2; ++i)
    {
        pid = fork();
        if(pid == 0)
            break;
    }

    if(i == 2) // 父进程，只有父进程才会执行到i == 2
    {
        int status;
        close(pipefd[0]);  // 关闭父进程的读端
        close(pipefd[1]);  // 关闭父进程的写端
        wpid = wait(&status);
        printf("child %d process exit with %d\n", wpid, status);
        wpid = wait(&status);
        printf("child %d process exit with %d\n", wpid, status);
    }
    else if(i == 0) // 创建的第一个子进程
    {
        sleep(1);
        // 子进程1--写管道
        close(pipefd[0]);  // 关闭子进程1的读端
        dup2(pipefd[1], STDOUT_FILENO); // 将管道的写端重定向到标准输出
        execlp("ls", "ls", "-l", NULL); // 执行ls -l命令
    }
    else if(i == 1) // 创建的第二个子进程
    {
        sleep(1);
        // 子进程2--读管道
        close(pipefd[1]);  // 关闭子进程2的写端
        dup2(pipefd[0], STDIN_FILENO); // 将管道的读端重定向到标准输入
        execlp("wc", "wc", "-l", NULL); // 执行wc -l命令
    }
    return 0;
}
```

> 匿名管道不可共用，因此父进程中管道的文件描述符必须要关闭，否则父进程中的读端会使进程阻塞。
>
> - 管道采用半双工通信方式，只能进行单向数据传输，如要实现同时双向通信，需要创建两个管道
> - 只有指向管道读端的文件描述符打开时，向管道中写入数据才有意义，否则写端会收到SIGPIPE信号，默认情况下该信号会导致进程终止
> - 若所有指向管道写端的文件描述符都被关闭后仍有进程从管道的读端读取数据，则管道中剩余的数据都被读取后，再次read()会返回EOF（文件结束符），即0
> - 若有指向管道写端的文件描述符未关闭，并且管道写端的进程也没有向管道中写入数据，那么当管道的读端进程读取数据且管道中剩余的数据都被读取后，再次read()会阻塞，直到管道中有数据可读或者所有指向管道写端的文件描述符都被关闭。当管道中没有数据可供读取，而读端进程正处于阻塞状态时，如果此时关闭所有指向管道写端的文件描述符，那么读端进程将会立即收到一个信号，这个信号会导致阻塞状态被打破，从而读取进程会从阻塞状态中返回，并且从读取系统调用中得到一个错误码，表明管道已经被关闭。
> - 若有指向管道读端的文件描述符没关闭，但读端进程没有从管道中读取数据，写端进程持续向管道中写入数据，那么管道缓存区写满时再次write会阻塞，直到读端将数据读出，阻塞才会解除。
> - 管道中的数据以字节流的形式传输，这要求管道两端的进程事先约定好数据的格式，否则会出现数据解析错误的情况。

### `popen()`和`pclose()`函数

> `popen()`函数可以创建一个管道，并返回一个I/O文件指针，该文件指针可以用于读取管道中的数据。`popen()`函数存在于`stdio.h`头文件中，其原型如下：
>
> ```c
> FILE *popen(const char *command, const char *type);
> ```
>
> `popen()`函数的功能是：调用`pipe()`函数创建一个管道，调用`fork()`函数创建子进程，然后在子进程中通过`execve()`函数调用Shell命令执行相应功能。若整个流程都成功执行，则返回一个I/O文件指针，若流程出现错误，则返回NULL。
>
> - `command`：一个字符串，用于指定子进程要执行的Shell命令
> - `type`：一个字符串，用于指定文件指针指向的文件连接到管道的哪一端，其取值如下：
>   - 若`type`设为`r`，文件连接到管道的读端，子进程的标准输出连接到管道的写端
>   - 若`type`设为`w`，文件连接到管道的写端，子进程的标准输入连接到管道的读端
>
> `pclose()`函数用于关闭由`popen()`函数打开的I/O文件，`pclose()`函数存在于`stdio.h`头文件中，其原型如下：
>
> ```c
> int pclose(FILE *stream);
> ```
>
> - `stream`：一个I/O文件指针，该指针指向由`popen()`函数打开的I/O文件，并通过调用`wait()`函数等待子进程命令执行结束，返回Shell的终止状态，防止产生僵尸进程。
> - `返回值`：若执行成功，则返回Shell的终止状态，若执行失败，则返回-1。

#### `popen()`和`pclose()`程序示例

```c
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define BUFFER_SIZE 1024

int main()
{
    FILE *r_fp, *w_fp;   // 定义I/O文件指针
    char buf[BUFFER_SIZE]; // 定义缓冲区

    /*
    第一个popen()函数的功能，先调用pipe()函数创建一个管道，然后调用fork()函数创建子进程，子进程通过execve()函数执行"ls -l"命令，子进程的标准输出连接管道的写端，将命令的执行结果（标准输出显示的内容）写入管道，父进程返回一个I/O文件指针，该文件指针指向的一个文件连接管道的读端可以用于读取管道中的数据。
    */
    r_fp = popen("ls -l", "r");    // 执行ls -l命令，创建一个管道，返回一个文件指针
    /*
    第二个popen()函数的功能，先调用pipe()函数创建一个管道，然后调用fork()函数创建子进程，子进程通过execve()函数执行"wc -l"命令，子进程的标准输入连接管道的读端，将管道中读取的内容交给子进程的标准输入，并由 wc -l 命令（在子进程中）处理，父进程返回一个I/O文件指针，该文件指针指向的一个文件连接管道的写端可以用于向管道中写入数据。
    */
    w_fp = popen("wc -l", "w");    // 执行wc -l命令，创建一个管道，返回一个文件指针

    if(r_fp == NULL || w_fp == NULL)
    {
        perror("popen error");
        exit(EXIT_FAILURE);
    }

    // 从读文件中读取数据
    while(fgets(buf, sizeof(buf), r_fp) != NULL)
    {
        // 将读文件中已保存到缓冲区中的数据写入写文件，写文件会将数据写入管道
        fputs(buf, w_fp);
    }
    
    // 关闭文件指针
    pclose(r_fp);
    pclose(w_fp);
    return 0;
}
```

### 命名管道（FIFO）

> 命名管道（也称为 FIFO，即 First In First Out）是一种用于进程间通信的机制。命名管道与普通管道（匿名管道）类似，允许数据按照先进先出的顺序传输，但与匿名管道不同的是，命名管道在文件系统中有对应的路径名，以文件的形式存在于文件系统中。命名管道可以由一个进程创建，并被任何知道其名称的进程打开进行读取或写入。这种方式的通信不限于有亲缘关系的进程（例如父子进程），任何进程都可以访问同一个命名管道来进行数据通信。FIFO对应的文件没有数据块，其本质和匿名管相同，都是由内核管理的一块缓存，读写不会改变文件的大小。缓冲区为空时，读操作会阻塞，缓冲区满时，写操作会阻塞。
>
> 在`Shell`中，可以使用`mkfifo`命令创建命名管道，其原型如下：
>
> ```shell
> mkfifo [option] fifo_name
> ```
>
> 在程序中，可以使用`mkfifo()`函数创建命名管道，在头文件`sys/types.h`和`sys/stat.h`中，其原型如下：
>
> ```c
> int mkfifo(const char *pathname, mode_t mode);
> ```
>
> - `pathname`：一个字符串，用于指定要创建的命名管道的路径名
> - `mode`：一个整型数，用于指定创建的命名管道的权限
> - `返回值`：`mkfifo()`函数成功时返回0，失败时返回-1。

#### 命名管道程序示例

```c
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
    sem = sem_open("my_sem", O_CREAT, 0664, 0);

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
```

```c
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
```

## 2.消息队列

> 使用消息队列实现进程间通信的步骤如下：
>
> 1. 创建消息队列
> 2. 发送消息到消息队列
> 3. 从消息队列中读取消息
> 4. 删除消息队列
>
> Linux内核提供了4个系统调用用于实现以上步骤，分别是`msgget()`、`msgsnd()`、`msgrcv()`和`msgctl()`，这4个系统调用存在于头文件`sys/msg.h`中。
>

### `msgget()`函数

> **`msgget()`函数用于创建消息队列或者获取一个已经存在的消息队列，其原型如下：**
>
> ```c
> int msgget(key_t key, int msgflg);
> ```
>
> - `key`：一个整型数，用于指定消息队列的键值，若键值为IPC_PRIVATE，将会获得一个只能被创建该消息队列的进程所读写的消息队列
> - `msgflg`：一个整型数，用于指定消息队列的权限和创建方式。
>   - 当`msgflg = mask | IPC_CREAT`时，若消息队列不存在，则创建一个消息队列，若消息队列已经存在，则返回该消息队列的标识符。
>   - 当`msgflg = mask | IPC_CREAT | IPC_EXCL`时，若消息队列不存在，则创建一个消息队列，若消息队列已经存在，则返回-1并设置`errno`为`EEXIST`。
> - `返回值`：若创建消息队列成功，则返回消息队列的标识符，若创建消息队列失败，则返回-1并设置`errno`。

### `msgsnd()`函数

> **`msgsnd()`函数用于向指定消息队列中发送消息，其原型如下：**
>
> ```c
> int msgsnd(int msqid, const void *msgp, size_t msgsz, int msgflg);
> ```
>
> - `msqid`：一个整型数，用于指定消息队列的标识符；
> - `msgp`：一个指向`msgbuf`结构体（消息缓冲区）的指针，用于指定要发送的消息；
>   `msgsnd()`函数发送消息有两个约束
>
>   - 消息长度必须小于系统规定上限，否则会返回错误码`EMSGSIZE`。
>   - 消息必须以一个长整型成员变量开始，因为要利用此变量先确定消息类型。  
>
> Linux系统中定义了一个结构体`msgbuf`，用于存储消息，其原型如下：
>
> ```c
> struct msgbuf
> {
>     long mtype; // 消息类型
>     char mtext[1];  // 消息正文
> };
> ```
>
> - `msgsz`：一个整型数，用于指定要发送的消息的数据长度，该长度不包括`msgbuf`结构体成员变量`mtype`的长度；
> - `msgflg`：一个整型数，可以设置为`0`或`IPC_NOWAIT`，若消息队列已满或系统中的消息数量达到上限
>   - 当`msgflg = 0`时，则发送进程将会阻塞，直到消息队列中有空闲空间；
>   - 当`msgflg = IPC_NOWAIT`时，则发送进程不会阻塞，而是立即返回错误码`EAGAIN`
> - `返回值`：若发送消息成功返回消息队列的标识符，失败返回-1并设置errno。

### `msgrcv()`函数

> **`msgrcv()`函数用于从指定消息队列中读取消息，被读取的消息会从消息队列中移除，其原型如下：**
>
> ```c
> ssize_t msgrcv(int msqid, void *msgp, size_t msgsz, long msgtyp, int msgflg);
> ```
>
> 若该函数调用成功，则返回消息正文的长度，失败时返回-1并设置errno。
>
> - `msqid`：表示消息队列的标识符，由`msgget()`函数返回；
> - `msgp`：一个指向`msgbuf`结构体（消息缓冲区）的指针，用于存储读取到的消息；
> - `msgsz`：表示消息的长度，该长度不包括`msgbuf`结构体中成员变量`mtype`的长度；
> - `msgtyp`：表示从消息队列中读取的消息类型，其取值如下：
>   - `msgtyp > 0`，表示读取消息队列中类型为msgtyp的第一条消息；
>   - `msgtyp = 0`，表示读取消息队列中的第一条可用消息；
>   - `msgtyp < 0`，表示读取消息队列中类型值小于或等于msgtyp绝对值的第一条消息；
> - `msgflg`：表示读取消息的方式，其取值如下：
>   - `msgflg = 0`，表示读取消息时，若消息队列中没有消息，则阻塞，直到消息队列中有消息；
>   - `msgflg = IPC_NOWAIT`，表示读取消息时，若消息队列中没有消息，则立即返回错误码`ENOMSG`；
>   - `msgflg = IPC_EXCEPT`，表示读取消息时，若消息队列中没有类型值等于msgtyp的消息，则阻塞，直到消息队列中有类型值等于msgtyp的消息；

### `msgctl()`函数

> **`msgctl()`函数对指定消息队列进行控制，其原型如下：**
>
> ```c
> int msgctl(int msqid, int cmd, struct msqid_ds *buf);
> ```
>
> - `msqid`：表示消息队列的标识符，由msgget()函数返回；
> - `cmd`：表示要执行的操作，其取值如下：
>   - `cmd = IPC_STAT`，表示获取消息队列的状态，此时参数`buf`指向一个`msqid_ds`结构体，用于存储消息队列的状态；
>   - `cmd = IPC_SET`，表示设置消息队列的状态，此时参数`buf`指向一个`msqid_ds`结构体，用于设置消息队列的状态；
>   - `cmd = IPC_RMID`，表示删除消息队列；
> - `buf`是一个指向`msqid_ds`结构体（缓冲区）的指针，用于存储消息队列的状态或设置消息队列的状态。内核为每个消息队列维护一个`msqid_ds`结构体，该结构体定义在`sys/ipc.h`中，函数原型如下：
>
> ```c
> struct msqid_ds
> {
>     struct ipc_perm msg_perm;   // 消息队列的所有权和权限
>     time_t msg_stime;   // 最后一次发送消息的时间
>     time_t msg_rtime;   // 最后一次接收消息的时间
>     time_t msg_ctime;   // 最后一次变更消息队列的时间
>     unsigned long msg_cbytes;   // 消息队列中的字节数
>     unsigned long msg_qnum; // 消息队列中的消息数量
>     unsigned long msg_qbytes;   // 消息队列的最大字节数
>     pid_t msg_lspid;    // 最后一次发送消息的进程ID
>     pid_t msg_lrpid;    // 最后一次接收消息的进程ID
> };
>
> struct ipc_perm
> {
>     key_t __key;    // 消息队列的键值
>     uid_t uid;  // 消息队列的所有者的用户ID
>     gid_t gid;  // 消息队列的所有者的组ID
>     uid_t cuid; // 消息队列的创建者的用户ID
>     gid_t cgid; // 消息队列的创建者的组ID
>     unsigned short mode;   // 消息队列的访问权限
>     unsigned short __seq;  // 消息队列的序列号
> };
> ```

#### 消息队列程序示例

```c
// msg_send.c
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <string.h>

#define MAX_MSG_SIZE 1024

// 定义消息结构体
struct msg_buf
{
    long msg_type; // 消息类型
    char msg_text[MAX_MSG_SIZE];   // 消息正文
};

int main()
{
    int msgid;  // 定义消息队列标识符
    struct msg_buf msg; // 定义消息结构体
    key_t key;  // 定义键值

    // 生成一个唯一的键值
    /*
    ftok 函数是用于生成一个唯一的键值，用于创建或获取 System V IPC 对象（比如消息队列、信号量、共享内存）的标识符。它的原型如下：
    key_t ftok(const char *pathname, int proj_id);
    ftok 函数用于将给定的文件路径和项目标识符转换为一个键值（key），以便用于创建或获取 System V IPC 对象的标识符。
    通过将文件路径和项目标识符转换为键值，可以确保不同的 System V IPC 对象可以使用不同的键值，从而避免冲突。
    ftok 函数接受两个参数，pathname 是一个指向文件路径的指针，proj_id 是项目标识符。
    ftok 将 pathname 参数指定的文件的设备号和 inode 号与 proj_id 组合起来，生成一个键值作为返回结果。
    生成的键值可以用于创建或获取消息队列、信号量和共享内存的标识符。
    */
    if((key = ftok(".", 'a')) == -1)
    {
        perror("ftok error");
        exit(EXIT_FAILURE);
    }

    // 创建消息队列
    if((msgid = msgget(key, IPC_CREAT | 0664)) == -1)
    {
        perror("msgget error");
        exit(EXIT_FAILURE);
    }

    // 发送消息
    while(1)
    {
        // 准备消息
        printf("Enter some message: ");
        fgets(msg.msg_text, MAX_MSG_SIZE, stdin); // 从标准输入读取数据
        msg.msg_type = 1;   // 设置消息类型
        // 发送消息
        msgsnd(msgid, (void *)&msg, MAX_MSG_SIZE, 0); // 发送消息
        if(strncmp(msg.msg_text, "exit", 4) == 0) // 如果输入exit，则退出
        {
            break;
        }
    }

    // 删除消息队列的工作交由接收消息的程序执行
    //if(msgctl(msgid, IPC_RMID, NULL) == -1)
    //{
    //    perror("msgctl error");
    //    exit(EXIT_FAILURE);
    //}
    return 0;
}
```

```c
// msg_recv.c
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <string.h>

#define MAX_MSG_SIZE 1024

// 定义消息结构体
struct msg_buf
{
    long msg_type; // 消息类型
    char msg_text[MAX_MSG_SIZE];   // 消息正文
};

int main()
{
    int msgid;  // 定义消息队列标识符
    struct msg_buf msg; // 定义消息结构体
    key_t key;  // 定义键值

    // 生成一个唯一的键值
    /*
    ftok 函数是用于生成一个唯一的键值，用于创建或获取 System V IPC 对象（比如消息队列、信号量、共享内存）的标识符。它的原型如下：
    key_t ftok(const char *pathname, int proj_id);
    ftok 函数用于将给定的文件路径和项目标识符转换为一个键值（key），以便用于创建或获取 System V IPC 对象的标识符。
    通过将文件路径和项目标识符转换为键值，可以确保不同的 System V IPC 对象可以使用不同的键值，从而避免冲突。
    ftok 函数接受两个参数，pathname 是一个指向文件路径的指针，proj_id 是项目标识符。
    ftok 将 pathname 参数指定的文件的设备号和 inode 号与 proj_id 组合起来，生成一个键值作为返回结果。
    生成的键值可以用于创建或获取消息队列、信号量和共享内存的标识符。
    */
    if((key = ftok(".", 'a')) == -1)
    {
        perror("ftok error");
        exit(EXIT_FAILURE);
    }

    // 获取消息队列
    if((msgid = msgget(key, IPC_CREAT | 0664)) == -1)
    {
        perror("msgget error");
        exit(EXIT_FAILURE);
    }

    // 接收消息
    while(1)
    {
        // 消息发送端的消息类型为1，接收端接收消息设置消息类型可以为0，表示接收队列中的第一条消息，或者为1，表示接收队列中类型为1的第一条消息
        msgrcv(msgid, (void *)&msg, MAX_MSG_SIZE, 0, 0); // 接收消息
        printf("Receive message: %s", msg.msg_text); // 打印消息
        if(strncmp(msg.msg_text, "exit", 4) == 0) // 如果接收到exit，则退出
        {
            break;
        }
        /*
        strncmp()函数是C语言中的一个字符串比较函数，它的作用是比较两个字符串的前n个字符。
        int strncmp(const char *str1, const char *str2, size_t n);
        其中，str1和str2是要比较的两个字符串，n是要比较的字符数。
        strncmp()函数会按字典顺序比较str1和str2的前n个字符。如果str1在str2之前，函数返回负数；如果str1在str2之后，函数返回正数；如果两个字符串相等，函数返回0。
        */

    }

    // 删除消息队列
    if(msgctl(msgid, IPC_RMID, NULL) == -1)
    {
        perror("msgctl error");
        exit(EXIT_FAILURE);
    }
    return 0;
}
```

## 3.信号量

在Linux中多个进程可能因为进程合作或资源共享而产生制约关系，分为直接制约关系和间接制约关系。

- 直接制约关系：进程A的执行依赖于进程B的执行，进程B的执行依赖于进程A的执行，这种关系称为直接制约关系。例如，利用管道机制实现进程间通信时，进程A向管道中写入数据，进程B从管道中读取数据，若管道存满时进程B不读取数据，进程A的写入操作会阻塞；若管道为空时进程A不写入数据，进程B的读取操作会阻塞，这种关系称为直接制约关系。
- 间接制约关系：进程A的执行依赖资源R，进程B的执行依赖资源R，资源R同一时刻只能被一个进程访问，这种关系称为间接制约关系。例如，当前系统中只有一台打印机，进程A和进程B都需要使用打印机，进程A使用打印机时，进程B就处于阻塞状态，类似于这种因资源共享导致的制约关系称为间接制约关系。  
  
同一时刻只允许有限个进程进行访问和修改的资源称为临界资源，**进程对临界资源的访问和修改必须互斥进行**，否则会导致数据不一致的问题。用于访问临界资源的代码称为临界区，临界区也属于临界资源，若能保证进程间互斥地进入临界区（同一时刻只允许有限个进程互斥地执行访问临界资源的代码），就能实现进程对临界资源的互斥访问。信号量是一种专门用于实现进程间同步与互斥的通信机制，其包括表示资源数量的非负整型变量、修改信号量的原子操作P和V，以及该信号量下等待资源的进程队列。

> 在Linux系统中，不同的进程通过获取同一个信号量键值进行通信，实现进程间的互斥。使用信号量进行通信时，通常需要以下步骤：
>
> 1. 创建或者获取系统中已有的信号量/信号量集。
> 2. 初始化信号量/信号量集。进程一次需要访问和修改多个同类的临界资源或多个不同类且不唯一的临界资源，需要初始化信号量集，否则只需要初始化一个信号量。
> 3. 信号量的P、V操作。进程在同步和互斥访问临界资源时，需要对信号量的数量进行修改。
> 4. 从系统中删除信号量/信号量集。
>
> 系统中信号量的数量是有限的，最大值由宏`SEMMSL`设定，Linux内核提供了3个系统调用用于实现以上步骤，分别是`semget()`、`semop()`和`semctl()`，这3个系统调用存在于头文件`sys/sem.h`中。
>

### `semget()`函数

> **`semget()`函数用于创建信号量集或者获取一个已经存在的信号量集，其原型如下：**
>
> ```c
> int semget(key_t key, int nsems, int semflg);
> ```
>
> - `key`：一个整型数，用于指定信号量集的键值；
> - `nsems`：一个整型数，用于指定信号量集中信号量的数量；
> - `semflg`：一个整型数，用于指定信号量集的权限和创建方式。
> - `返回值`：若创建信号量集成功，则返回信号量集的标识符，若创建信号量集失败，则返回-1并设置errno。常见的errno如下：
>   - `EACCES`：表示进程访问权限。
>   - `EEXIST`：当`semflg = IPC_CREAT | IPC_EXCL`时，表示信号量集已经存在。
>   - `EINVAL`：表示信号量集中的信号量数量小于0或者大于系统规定的上限。或者指定的`key`值相对应的信号量集已经存在，但是指定的信号量的数量（nsems）与已经存在的信号量集中的信号量数量不一致。
>   - `ENOENT`：表示指定的`key`值相对应的信号量集不存在，并且`semflg`中没有指定`IPC_CREAT`。
>
> 当创建一个新的信号量集时，`semget()`初始化与信号量集相关的数据结构semid_ds

### `semop()`函数

> **`semop()`函数用于对指定信号量集中的信号量进行P、V操作，其原型如下：**
>
> ```c
> int semop(int semid, struct sembuf *sops, size_t nsops);
> ```
>
> - `semid`：一个整型数，用于指定信号量集的标识符；
> - `sops`：一个指向`sembuf`结构体（信号量操作结构体）数组的指针，用于指定要进行的P、V操作；
> - `nsops`：一个整型数，指定参数`sops`指向的数组的大小。
>
> `sops`指向一个`struct sembuf`结构体的数组，数组大小由`nsops`指定，`struct sembuf`结构体定义在`sys/sem.h`中，其原型如下：
>
> ```c
> struct sembuf
> {
>     unsigned short sem_num; // 信号量集中信号量的编号
>     short sem_op;   // 信号量的操作
>     short sem_flg;  // 操作标志
> };
> ```
>
> - `sem_num`：表示信号量集中信号量的编号，编号从0开始；
> - `sem_op`：表示对该信号量的操作，其取值如下：
>   - `sem_op > 0`，表示对信号量执行V操作，即信号量的数量加上`sem_op`；
>   - `sem_op = 0`，表示对信号量执行Z操作，即等待信号量的数量变为0；
>   - `sem_op < 0`，表示对信号量执行P操作，即信号量的数量减去`sem_op`；
> - `sem_flg`：表示的操作标志，其取值如下：
>   - `sem_flg = 0`，表示对信号量执行操作时，若信号量的数量不满足操作的条件，则阻塞，直到信号量的数量满足操作的条件；
>   - `sem_flg = IPC_NOWAIT`，表示对信号量执行操作时，若信号量的数量不满足操作的条件，则立即返回错误码`EAGAIN`；
>   - `sem_flg = SEM_UNDO`，表示希望在进程终止时自动撤销对信号量的操作。当进程因为某种原因意外终止时，系统会自动执行撤销操作，将信号量的值恢复到操作之前的状态，以防止由于进程意外终止而导致的资源泄露或者死锁情况。那么系统会维护一个信号量调整值（semadj），在进程对信号量进行操作时，系统会将sem_op的值加减到semadj中。当进程意外终止时，系统会自动从semadj中加减去之前对信号量的操作值，以将信号量的值恢复到操作之前的状态。

### `semctl()`函数

> **`semctl()`函数用于对指定信号量或信号量集进行控制，其原型如下：**
>
> ```c
> int semctl(int semid, int semnum, int cmd, ...);
> ```
>
> - `semid`：一个整型数，用于指定信号量集的标识符；
> - `semnum`：一个整型数，用于指定信号量集中信号量的编号（编号从0开始）；
> - `cmd`：一个整型数，用于指定要执行的操作，其取值如下：
>   - `cmd = IPC_STAT`，表示获取信号量集的状态，此时第四个参数`arg`指向一个`semid_ds`结构体，用于存储信号量集的状态，第二个参数`semnum`将被忽略；
>   - `cmd = IPC_SET`，表示设置信号量集的状态，此时第四个参数`arg`指向一个结构体成员变量被写入值后的`semid_ds`结构体，用于设置信号量集的状态；
>   - **`cmd = IPC_RMID`，表示立即删除指定信号量集，唤醒所有在该信号量集上调用`semop()`函数而处于阻塞状态的进程，并返回`EIDRM`的errno；**
> - `cmd = GETALL`，表示获取信号量集中所有信号量的值，此时参数arg指向一个short型数组，用于存储信号量集中所有信号量的值；
> - `cmd = SETALL`，表示设置信号量集中所有信号量的值，此时参数arg指向一个short型数组，用于设置信号量集中所有信号量的值；
> - `cmd = GETNCNT`，表示获取信号量集中等待信号量的进程数量；
> - `cmd = GETPID`，表示获取信号量集中最后一个执行semop()函数的进程的进程ID；
> - `cmd = GETVAL`，表示获取信号量集中指定信号量的值，此时参数arg指向一个short型变量，用于存储信号量的值；
> - `cmd = GETZCNT`，表示获取信号量集中等待信号量的进程数量；
> **- cmd = SETVAL，表示设置信号量集中指定信号量的值，此时参数arg指向一个short型变量，用于设置信号量的值；**
> - `arg`：一个可变参数形参，形参类型在联合体`semun`中，其类型取决于参数cmd的取值，`union semun`其原型如下：
>
> ```c
> union semun
> {
>     int val;    // 用于SETVAL命令
>     struct semid_ds *buf;   // 用于IPC_STAT和IPC_SET命令
>     unsigned short *array;  // 用于GETALL和SETALL命令
>     struct seminfo *__buf;  // 用于IPC_INFO命令
> };
> ```
>
> `semid_ds`结构体定义在`sys/sem.h`中，其原型如下：
>
> ```c
> struct semid_ds
> {
>     struct ipc_perm sem_perm;   // 信号量集的所有权和权限
>     time_t sem_otime;   // 最后一次执行semop()函数的时间
>     time_t sem_ctime;   // 最后一次变更信号量集的时间
>     unsigned short sem_nsems;   // 信号量集中信号量的数量
> };
> ```
>
> `sem_perm`表示信号量集的所有权和权限，其类型为`ipc_perm`结构体，定义在`sys/ipc.h`中，其原型如下：
>
> ```c
> struct ipc_perm
> {
>     key_t __key;    // 信号量集的键值
>     uid_t uid;  // 信号量集的拥有者的用户ID
>     gid_t gid;  // 信号量集的拥有者的组ID
>     uid_t cuid; // 信号量集的创建者的用户ID
>     gid_t cgid; // 信号量集的创建者的组ID
>     unsigned short mode;   // 信号量集的权限
>     unsigned short __seq;  // 信号量集的序列号
> };
> ```

#### 信号量程序示例

```c
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

// 定义联合体用于semctl初始化
union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    struct seminfo *__buf;
};

int main() {
    key_t key;
    int semid;
    union semun sem_union;
    struct sembuf sem_b;

    // 生成一个唯一的键值，通常情况下，ftok的参数为当前目录和一个项目ID
    key = ftok(".", 'a');
    if (key == -1) {
        perror("ftok error");
        exit(1);
    }

    // 创建一个信号量集，包含1个信号量
    semid = semget(key, 1, 0666 | IPC_CREAT);
    if (semid == -1) {
        perror("semget error");
        exit(1);
    }

    // 初始化信号量，设置其值为1
    sem_union.val = 1;
    if (semctl(semid, 0, SETVAL, sem_union) == -1) {
        perror("semctl error");
        exit(1);
    }

    // 对信号量执行减1操作，即P操作
    sem_b.sem_num = 0;  // 信号量集中的第一个信号量
    sem_b.sem_op = -1;  // P操作
    sem_b.sem_flg = SEM_UNDO;  // 系统自动释放信号量
    if (semop(semid, &sem_b, 1) == -1) {
        perror("semop error");
        exit(1);
    }

    // 临界区，可以执行对共享资源的访问
    printf("Entered critical section\n");

    // 退出临界区前，执行加1操作，即V操作
    sem_b.sem_op = 1;  // V操作
    if (semop(semid, &sem_b, 1) == -1) {
        perror("semop error");
        exit(1);
    }

    printf("Exited critical section\n");

    // 删除信号量集
    if (semctl(semid, 0, IPC_RMID, sem_union) == -1) {
        perror("semctl error");
        exit(1);
    }

    return 0;
}
```

```c
// sem_create.c
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdlib.h>
#include <string.h>

// 定义信号量联合体
union semun
{
    int val;    // 用于SETVAL命令
    struct semid_ds *buf;   // 用于IPC_STAT和IPC_SET命令
    unsigned short *array;  // 用于GETALL和SETALL命令
    struct seminfo *__buf;  // 用于IPC_INFO命令
};

static int sem_id;  // 定义信号量标识符

static int set_semvalue(); // 定义设置信号量值的函数
{
    union semun sem_union;  // 定义信号量联合体
    sem_union.val = 1;  // 设置信号量的值为1
    if(semctl(sem_id, 0, SETVAL, sem_union) == -1) // 设置信号量的值
    {
        return 0;
    }
    return 1;
}

static void del_semvalue(); // 定义删除信号量的函数
{
    union semun sem_union;  // 定义信号量联合体
    if(semctl(sem_id, 0, IPC_RMID, sem_union) == -1) // 删除信号量
    {
        perror("Failed to delete semaphore\n");
    }
}

static int semaphore_p(); // 定义P操作的函数
{
    struct sembuf sem_b;    // 定义信号量操作结构体
    sem_b.sem_num = 0;  // 信号量集中信号量的编号为0
    sem_b.sem_op = -1;  // 执行P操作
    sem_b.sem_flg = SEM_UNDO;   // 设置信号量的标志
    if(semop(sem_id, &sem_b, 1) == -1) // 执行P操作
    {
        perror("semaphore_p failed\n");
        return 0;
    }
    return 1;
}

static int semaphore_v(); // 定义V操作的函数
{
    struct sembuf sem_b;    // 定义信号量操作结构体
    sem_b.sem_num = 0;  // 信号量集中信号量的编号为0
    sem_b.sem_op = 1;   // 执行V操作
    sem_b.sem_flg = SEM_UNDO;   // 设置信号量的标志
    if(semop(sem_id, &sem_b, 1) == -1) // 执行V操作
    {
        perror("semaphore_v failed\n");
        return 0;
    }
    return 1;
}

int main(int argc, char **argv)
{
    char message = 'X'; // 定义消息
    int i = 0;
    // 创建信号量
    sem_id = semget((key_t)1234, 1, 0666 | IPC_CREAT);
    if(argc > 1)
    {
        // 程序第一次被调用，初始化信号量
        if(!set_semvalue())
        {
            fprintf(stderr, "Failed to initialize semaphore\n");
            exit(EXIT_FAILURE);
        }
        // 设置要输出到屏幕中的信息，即其参数的第一个字符
        message = argv[1][0];
        sleep(2);
    }
    for(i = 0; i < 10; i++)
    {
        // 进入临界区
        if(!semaphore_p())
        {
            exit(EXIT_FAILURE);
        }
        // 向屏幕中输出数据
        printf("%c", message);
        // 清理缓冲区，然后休眠随机时间
        fflush(stdout);
        sleep(rand() % 3);
        // 离开临界区前再一次向屏幕输出数据
        printf("%c", message);
        fflush(stdout);
        // 离开临界区，休眠随机时间后继续循环
        if(!semaphore_v())
        {
            exit(EXIT_FAILURE);
        }
        sleep(rand() % 2);
    }
    sleep(10);
    printf("\n%d - finished\n", getpid());
    if(argc > 1)
    {
        // 如果程序是第一次被调用，则在退出前删除信号量
        sleep(3);
        del_semvalue();
    }
    exit(EXIT_SUCCESS);
}
```

## 4.共享内存

> Linux内核提供了一些系统调用用于实现共享内存的申请、管理与释放，分别为`shmget()`、`shmat()`、`shmdt()`和`shmctl()`，这4个系统调用存在于头文件`sys/shm.h`中。

### `shmget()`函数

> **`shmget()`函数用于创建共享内存段或者获取一个已经存在的共享内存段，其原型如下：**
>
> ```c
> int shmget(key_t key, size_t size, int shmflg);
> ```
>
> - `key`：一个整型数，用于指定共享内存段的键值；
> - `size`：一个整型数，用于指定共享内存段的大小（以字节为单位）；
> - `shmflg`：一个整型数，用于指定共享内存段的权限和创建方式。
>   - `IPC_CREAT` 表示如果指定的键不存在，则创建一个新的共享内存段。
>   - `IPC_EXCL` 与 `IPC_CREAT` 一起使用时，如果共享内存已存在，则返回错误。
> - `返回值`：若创建共享内存段成功，则返回共享内存段的标识符，若创建共享内存段失败，则返回-1并设置errno。man 2 shmget 查看errno的值。

### `shmat()`函数

> **shmat()函数用于将共享内存段映射到进程的虚拟地址空间，其原型如下：**
>
> ```c
> void *shmat(int shmid, const void *shmaddr, int shmflg);
> ```
>
> - `shmid`：一个整型数，用于指定共享内存段的标识符；
> - `shmaddr`：一个指向共享内存段的虚拟地址的指针；
>   - 若`shmaddr = NULL`，则由系统自动选择一个合适的地址；
> - `shmflg`：一个整型数，用于指定共享内存段的映射方式，其取值如下：
>   - `shmflg = 0`，表示共享内存段的映射方式为读写；
>   - `shmflg = SHM_RDONLY`，表示共享内存段的映射方式为只读，当前进程只能读取共享内存段中的数据，不能修改共享内存段中的数据；
> - `返回值`：若映射成功，则返回共享内存段的虚拟地址，若映射失败，则返回-1并设置errno。

### `shmdt()`函数

> **`shmdt()`函数用于将共享内存段从进程的虚拟地址空间中分离，即解除物理内存中共享内存的地址与进程虚拟地址空间中某个虚拟地址的映射关系，其原型如下：**
>
> ```c
> int shmdt(const void *shmaddr);
> ```
>
> - `shmaddr`：一个指向共享内存段的虚拟地址的指针，该指针必须是共享内存段的起始地址。The to-be-detached shared memory segment must be currently attached with shmaddr equal to the value returned by the attaching shmat() call.
> - `返回值`：若成功，则返回0，若失败，则返回-1并设置errno。On a successful shmdt() call, the system updates the members of the shmid_ds structure associated with the shared memory segment.

### `shmctl()`函数

> **`shmctl()`函数用于对指定共享内存段进行控制，其原型如下：**
>
> ```c
> int shmctl(int shmid, int cmd, struct shmid_ds *buf);
> ```
>
> - `shmid`：一个整型数，用于指定共享内存段的标识符；
> - `cmd`：一个整型数，用于指定要执行的操作，其取值如下：
>   - `cmd = IPC_STAT`，表示获取共享内存段的状态，此时参数`buf`指向一个`shmid_ds`结构体，用于存储共享内存段的状态；
>   - `cmd = IPC_SET`，表示设置共享内存段的状态，此时参数`buf`指向一个`shmid_ds`结构体，用于设置共享内存段的状态；
>   - `cmd = IPC_RMID`，表示删除共享内存段；
> - `buf`：一个指向`shmid_ds`结构体（缓冲区）的指针，用于存储共享内存段的状态或设置共享内存段的状态。内核为每个共享内存段维护一个`shmid_ds`结构体，该结构体定义在`sys/shm.h`中，函数原型如下：
>
> ```c
> struct shmid_ds
> {
>    struct ipc_perm shm_perm;   // 共享内存段的所有权和权限
>    size_t shm_segsz;   // 共享内存段的大小
>    time_t shm_atime;   // 最后一次执行shmat()函数的时间
>    time_t shm_dtime;   // 最后一次执行shmdt()函数的时间
>    time_t shm_ctime;   // 最后一次变更共享内存段的时间
>    pid_t shm_cpid; // 创建共享内存段的进程ID
>    pid_t shm_lpid; // 最后一次执行shmat()或shmdt()函数的进程ID
>    shmatt_t shm_nattch;    // 共享内存段的当前连接数
> };
> ```
>
> - `返回值`： A successful SHM_STAT operation returns
       the identifier of the shared memory segment whose index was given in shmid.  Other operations return 0 on success. On error, -1 is returned, and errno is set appropriately.

#### 共享内存程序示例

```c
// shm_write.c
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <unistd.h>

#define SHMSIZE 4096    // 定义共享内存段的大小 4KB

int main()
{r
    int shm_id; // 定义共享内存段的标识符
    key_t key;  // 定义键值
    char *shmaddr;    // 定义指向共享内存段的虚拟地址指针
  

    // 生成一个唯一的键值
    key = ftok(".", 'a');
    if(key == -1)
    {
        perror("ftok error");
        exit(EXIT_FAILURE);
    }
    printf("key = %d\n", key);

    // 创建共享内存段
    shm_id = shmget(key, SHMSIZE, IPC_CREAT | IPC_EXCL | 0664);
    if(shm_id == -1)
    {
        perror("shmget error");
        exit(EXIT_FAILURE);
    }
    printf("shm_id = %d\n", shm_id);

    // 将共享内存段映射到进程的虚拟地址空间
    shmaddr = (char*)shmat(shm_id, NULL, 0);
    if(shmaddr == (char*)-1) // (char *)-1 = 0xFFFFFFFF
    {
        perror("shmat error");
        exit(EXIT_FAILURE);
    }
    // 虚拟地址空间中的shmadd通过MMU映射到物理内存中的共享内存段
    memset(shmaddr, '\0', SHMSIZE);  // 将共享内存段清空 '\0' = 0x00

    strcpy(shmaddr, "Hi, I am writer process\n"); // 向共享内存段中写入数据

    // 等待读进程读取数据
     printf("Waiting for reader...\n");
    while (shmaddr[0] != '*') // 等待读进程读取数据后，向共享内存段中首字节的位置写入数据'*' 表示读进程已经读取数据 
    { 
        sleep(3);
    }

    // 解除共享内存段的映射关系
    if(shmdt(shmaddr) == -1)
    {
        perror("shmdt error");
        exit(EXIT_FAILURE);
    }

    // 删除共享内存段
    if(shmctl(shm_id, IPC_RMID, NULL) == -1)
    {
        perror("shmctl error");
        exit(EXIT_FAILURE);
    }
    return 0;
}
```

```c
// shm_read.c
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <unistd.h>

#define SHMSIZE 4096    // 定义共享内存段的大小 4KB

int main()
{
    key_t key;  // 定义键值
    int shm_id; // 定义共享内存段的标识符
    char *shmaddr;    // 定义指向共享内存段的指针

    // 生成一个唯一的键值
    key = ftok(".", 'a');
    if(key == -1)
    {
        perror("ftok error");
        exit(EXIT_FAILURE);
    }

    // 获取已经创建的共享内存段
    shm_id = shmget(key, SHMSIZE, IPC_CREAT | 0664);
    if(shm_id == -1)
    {
        perror("shmget error");
        exit(EXIT_FAILURE);
    }

    // 将共享内存段映射到进程的虚拟地址空间
    shmaddr = (char*)shmat(shm_id, NULL, 0);
    if(shmaddr == (char*)-1) // (char *)-1 = 0xFFFFFFFF
    {
        perror("shmat error");
        exit(EXIT_FAILURE);
    }   
    
    // 虚拟地址空间中的shmaddr通过MMU映射到物理内存中的共享内存段
    printf("shmaddr = %p\n", shmaddr);
    // 从共享内存段中读取数据
    printf("Data read from shared memory shmaddr = %s\n", shmaddr);

    // 向共享内存段中写入数据
    printf("Please input a string: ");
    *shmaddr = '*';  // *shmaddr == shmaddr[0] 首字节的地址值再取值

    // 解除共享内存段的映射关系
    if(shmdt(shmaddr) == -1)
    {
        perror("shmdt error");
        exit(EXIT_FAILURE);
    }

    return 0;

}
```

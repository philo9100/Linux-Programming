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
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

static int set_semvalue() // 定义设置信号量值的函数
{
    union semun sem_union;  // 定义信号量联合体
    sem_union.val = 1;  // 设置信号量的值为1
    if(semctl(sem_id, 0, SETVAL, sem_union) == -1) // 设置信号量的值
    {
        return 0;
    }
    return 1;
}

static void del_semvalue() // 定义删除信号量的函数
{
    union semun sem_union;  // 定义信号量联合体
    if(semctl(sem_id, 0, IPC_RMID, sem_union) == -1) // 删除信号量
    {
        perror("Failed to delete semaphore\n");
    }
}

static int semaphore_p() // 定义P操作的函数
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

static int semaphore_v() // 定义V操作的函数
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
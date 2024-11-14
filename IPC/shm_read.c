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
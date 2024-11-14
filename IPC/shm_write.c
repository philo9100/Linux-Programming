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
{
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
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
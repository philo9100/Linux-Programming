// pthread_mutex.c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>

pthread_mutex_t mutex; // 定义互斥锁变量

void* thread_func(void* arg)
{
    srand(time(NULL));
    while(1)
    {
        // 加锁互斥锁
        pthread_mutex_lock(&mutex);         // 加锁 mutex--
        printf("%ld :hello", pthread_self());
        // 模拟长时间操作共享资源的并发环境，让系统调度其他线程上cpu执行
        sleep(rand()%3);
        printf("%ld :world\n", pthread_self());
        // 解锁互斥锁
        pthread_mutex_unlock(&mutex);       // 解锁 mutex++
        sleep(rand()%3);
    }
    pthread_exit(NULL);
}

int main()
{
    pthread_t mythread1, mythread2;     // 定义线程标识符
    srand(time(NULL));

    // 初始化互斥锁变量
    pthread_mutex_init(&mutex, NULL);   // mutex = 1

    // 创建线程
    pthread_create(&mythread1, NULL, thread_func, NULL);
    pthread_create(&mythread2, NULL, thread_func, NULL);

    int i = 5;
    while(i--)
    {
        // 加锁互斥锁
        pthread_mutex_lock(&mutex);         // 加锁 mutex--
        printf("%ld: HELLO ", pthread_self());
        sleep(rand()%3);
        printf("%ld: WORLD\n", pthread_self());
        // 解锁互斥锁
        pthread_mutex_unlock(&mutex);       // 解锁 mutex++
        sleep(rand()%3);
    }

    pthread_cancel(mythread1);
    pthread_cancel(mythread2);

    pthread_join(mythread1, NULL);
    pthread_join(mythread2, NULL);

    // 销毁互斥锁变量
    pthread_mutex_destroy(&mutex);

    return 0;
}

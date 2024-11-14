// pthread_nomutex.c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

void* thread_func(void* arg)
{
    srand(time(NULL));
    while(1)
    {

        printf("%ld :hello", pthread_self());
        // 模拟长时间操作共享资源的并发环境，让系统调度其他线程上cpu执行
        sleep(rand()%3);
        printf("%ld :world\n", pthread_self());
        sleep(rand()%3);
    }
    pthread_exit(NULL);
}

int main()
{
    pthread_t mythread1, mythread2;     // 定义线程标识符

    srand(time(NULL));

    // 创建线程
    pthread_create(&mythread1, NULL, thread_func, NULL);
    pthread_create(&mythread2, NULL, thread_func, NULL);

    int i = 5;
    while(i--)
    {
        printf("%ld: HELLO ", pthread_self());
        sleep(rand()%3);
        printf("%ld: WORLD\n", pthread_self());
        sleep(rand()%3);
    }

    pthread_cancel(mythread1);
    pthread_cancel(mythread2);

    pthread_join(mythread1, NULL);
    pthread_join(mythread2, NULL);

    return 0;
}
/*
在这个示例中，thread_func1()通过调用pthread_exit()来退出线程
thread_func2()通过return来退出线程
thread_func3()通过调用exit()来退出整个进程。

你会注意到，当thread_func3()调用exit()时，整个进程都会立即结束，包括所有的线程。这就是为什么你不会看到"Main thread exiting"的输出。
*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>

void* thread1_func(void* arg)
{
    printf("thread1_func--thread1 is running\n");
    sleep(2);
    printf("thread1_func--thread1 is exiting\n");
    pthread_exit(NULL);
}

void* thread2_func(void* arg)
{
    printf("thread2_func--thread2 is running\n");
    sleep(2);
    printf("thread2_func--thread2 is exiting\n");
    return NULL;
}

void* thread3_func(void* arg)
{
    printf("thread3_func--thread3 is running\n");
    sleep(2);
    printf("thread3_func--thread3 is exiting\n");
    exit(EXIT_SUCCESS);
}

int main()
{
    pthread_t thread1, thread2, thread3;

    pthread_create(&thread1, NULL, thread1_func, NULL);
    pthread_create(&thread2, NULL, thread2_func, NULL);
    pthread_create(&thread3, NULL, thread3_func, NULL);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    pthread_join(thread3, NULL);

    printf("main thread--All threads are exited\n");

    return 0;
}
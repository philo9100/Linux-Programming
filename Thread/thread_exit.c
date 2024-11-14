#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

void* thread_fun(void *arg)
{
    int* ptr = (int*)malloc(sizeof(int)); // 申请内存

    *ptr = 666; // 给内存赋值

    // 之前都是通过return (void*)0语句来退出线程，这里我们通过pthread_exit()函数来退出线程，并将返回值传递给其他线程
    pthread_exit((void*)ptr); // 退出线程
    // 如果线程不需要返回值，则可以直接使用return语句退出线程，或者调用pthread_exit()函数退出线程。
}

int main()
{
    pthread_t mythread; // 线程标识符

    int ret;
    if(ret = pthread_create(&mythread, NULL, thread_fun, NULL))
    {
         fprintf(stderr, "pthread_create error: %s\n", strerror(ret));
        exit(EXIT_FAILURE);  
    }

    // 当前main线程要获取mythread线程的返回值，即一个void*类型的指针，那么就在此声明一个void*类型的指针，用于接收mythread线程执行函数的返回值，而此时result的内存空间应该是空的，需要把result的地址传给pthread_join()函数，让pthread_join()函数通过result的地址把mythread线程的返回值写入到result内存空间中。这样result指向的地址就和mythread线程的返回值指向同一个地址。
    void* result; // 用于接收线程执行函数的返回值，等同于线程执行函数的返回值

    // 等待线程结束，并获取线程的返回值
    if(ret = pthread_join(mythread, &result))
    {
        fprintf(stderr, "pthread_join error: %s\n", strerror(ret));
        exit(EXIT_FAILURE);  
    }

    // 在main线程中打印mythread线程中结果
    printf("main thread--mythread's result = %d\n", *(int*)result); // main thread--mythread's result = 666

    return 0;
}
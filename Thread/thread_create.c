#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>


// 线程执行函数
void *thread_fun(void *arg)
{
    /*
    在Linux中，gettid()函数是一个系统调用，用于获取当前线程的线程ID（TID）。
    然而，gettid()并没有在glibc中提供，所以不能直接在程序中调用它。

    如果想在程序中获取线程ID，你可以使用syscall(SYS_gettid)来调用gettid()系统调用。
    syscall()函数是在unistd.h头文件中定义的，SYS_gettid是在sys/syscall.h头文件中定义的。
    */
    pid_t tid = syscall(SYS_gettid); // 获取线程ID
    printf("thread_fun--getpid = %d, gettid = %d, pthread_self = %lu, pthread_t = %lu, arg = %ld\n", getpid(), tid, pthread_self(), (unsigned long)pthread_self(), *(pthread_t*)arg); 
    // thread_fun--getpid = 28578, gettid = 28579, pthread_self = 140338495387392, pthread_t = 140338495387392, arg = 140338495387392
    /*
    从上面的输出结果可以看出，线程ID和线程标识符是不同的，线程ID是系统分配的，而线程标识符是pthread_create()函数返回的。
    线程的pid是线程组长的pid，线程组长的pid和线程组长的线程ID是相同的。
    线程的线程表标识符pthread_t和pthread_self()函数的返回值是相同的，都是调用pthread_create()函数时传入的参数。
    */
    sleep(1);
    return (void*)0;    // (void*)0表示返回值为0x00 00 00 00 = NULL
}

int main()
{
    pthread_t mythread; // 线程标识符
    pid_t tid = syscall(SYS_gettid); // 获取线程ID

    int ret = pthread_create(&mythread, NULL, thread_fun, &mythread); // 创建线程，执行线程函数

    if (ret != 0) // 创建线程失败
    {
        fprintf(stderr, "pthread_create error: %s\n", strerror(ret));
        exit(EXIT_FAILURE);  
    }

    printf("main--getpid = %d, gettid = %d, pthread_self = %lu, pthread_t = %lu\n", getpid(), tid, pthread_self(), (unsigned long)pthread_self());
    // main--getpid = 28578, gettid = 28578, pthread_self = 140338503907136, pthread_t = 140338503907136
    /*
    从上面的输出结果可以看出，主线程（线程组长）的pid和线程组长的线程ID是相同的也是线程组的tgid
    主线程有自己的线程标识符pthread_t，并且线程表标识符pthread_t是pthread_self()函数的返回值。
    */
    sleep(2);

    // 等待线程结束
    ret = pthread_join(mythread, NULL);
    if (ret != 0) // 等待线程失败
    {
        fprintf(stderr, "pthread_join error: %s\n", strerror(ret));
        exit(EXIT_FAILURE);  
    }
    
    return 0;
}
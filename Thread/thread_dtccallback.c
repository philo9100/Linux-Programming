#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>

// int thread_num, int status
typedef void (*thread_exit_callback)(int, int); // 定义回调函数类型

// 全局互斥锁，用于同步对共享资源的访问
pthread_mutex_t lock;

// 线程回调函数
void on_thread_exit(int thread_num, int status)
{
    // 获取互斥锁
    pthread_mutex_lock(&lock);

    // 这里的代码是线程安全的，因为只有一个线程可以获取到互斥锁
    printf("Callback: Thread %d exited with status %d\n", thread_num, status);
    
    // 共享资源的更新或处理放在这里
    
    // 释放互斥锁
    pthread_mutex_unlock(&lock);
}

// 线程执行函数
void* thread_func(void* arg)
{
    thread_exit_callback callback =  (thread_exit_callback)arg; // 将arg强制转换成自定义回调函数类型

    pthread_t thread_self = pthread_self(); // 获取线程自身的线程标识符
    
    pthread_detach(thread_self); // 线程设置自分离状态，这也是pthread_detach()函数的通常用法，当然也可以在main()函数中调用pthread_detach()函数来设置线程分离状态

    // 将线程标识符当作线程的ID
    int thread_num = (int)thread_self; 

    printf("Thread %d is running\n", thread_num);

    // 模拟线程执行工作
    sleep(3);

    // 调用回调函数，并传递线程的状态
    callback(thread_num, thread_num * 2); // 
    
    // 退出线程
    pthread_exit(NULL);
}


int main()
{
    const int NUM_THREADS = 3; // 定义线程数量
    pthread_t mythread[NUM_THREADS]; // 线程标识符数组

    // 初始化互斥锁
    pthread_mutex_init(&lock, NULL);


    // 创建线程
    for(int i = 0; i < NUM_THREADS; ++i)
    {
        if(pthread_create(&mythread[i], NULL, thread_func, (void*)on_thread_exit) != 0)
        {
            perror("Failed to create the thread");
            // 清理互斥锁资源
            pthread_mutex_destroy(&lock);
            exit(EXIT_FAILURE);
        }
    }

    // 模拟主线程执行工作
    sleep(5);

    printf("Main thread exiting\n");

    // 销毁互斥锁
    pthread_mutex_destroy(&lock);

    return 0;
}
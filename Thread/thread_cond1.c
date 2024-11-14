#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// 定义一个全局变量，表明缓冲区中项的状态，0表示缓冲区空，1表示缓冲区不空
int buffer_has_item = 0;
pthread_mutex_t mutex;  // 定义互斥锁变量
pthread_cond_t cond;    // 定义条件变量

// 消费者函数
void* consumer(void *arg) {
    /*
    循环去执行消费者的任务，消费者线程和生产者线程是并行的，且消费者线程和生产者线程使用同一个互斥锁和条件变量
    */
    while (1) {
        pthread_mutex_lock(&mutex); // 如果消费者线程成功加锁，则生产者线程就会阻塞，如果有多个消费者线程，则其他消费者线程会阻塞
        while (buffer_has_item == 0) { // 循环等待条件变量，如果缓冲区为空，则消费者线程就会执行下面的wait函数进入阻塞，即不满足条件变量
            pthread_cond_wait(&cond, &mutex); // 等待条件变量，同时释放互斥锁，使得生产者线程可以有机会获得互斥锁，并且被唤醒后要再次判断并获取互斥锁，因为此时生产者可能还没释放锁，或者有多个消费者线程或生产者线程会先于被唤醒的消费者线程获得互斥锁
        }
        // 执行到这里说明满足条件变量，消费项...
        printf("消费者消费了一个项\n");
        buffer_has_item = 0; // 消费完后设置缓冲区为空
        pthread_mutex_unlock(&mutex);
        sleep(1); // 模拟消费者消费商品需要的时间
    }
    return NULL;
}

// 生产者函数
void* producer(void *arg) {
    while (1) {
        pthread_mutex_lock(&mutex); // 如果生产者线程成功加锁，则消费者线程就会阻塞，如果有多个生产者线程，则其他生产者线程会阻塞
        if (buffer_has_item == 0) { // 如果缓冲区为空，则生产者线程就会生产商品，如果缓冲区不为空，则直接释放互斥锁
            // 生产项...
            printf("生产者生产了一个项\n");
            buffer_has_item = 1; // 生产完后设置缓冲区不为空
            pthread_cond_signal(&cond); // 发送条件变量信号，唤醒因为条件变量不满足而阻塞的消费者线程
            // pthread_cond_broadcast(&cond); // 如果有多个消费者线程因为条件变量不满足而阻塞，生产者线程广播条件变量信号，唤醒所有因为条件变量不满足而阻塞的消费者线程
        }
        pthread_mutex_unlock(&mutex);
        sleep(2); // 模拟生产者生产商品需要的时间
    }
    return NULL;
}

int main() {
    pthread_t prod, cons;

    // 初始化互斥锁和条件变量
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    // 创建生产者和消费者线程
    pthread_create(&prod, NULL, producer, NULL);
    pthread_create(&cons, NULL, consumer, NULL);

    // 等待线程结束
    pthread_join(prod, NULL);
    pthread_join(cons, NULL);

    // 销毁互斥锁和条件变量
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    return 0;
}
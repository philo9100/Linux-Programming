// pthread_cond.c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // 定义并初始化互斥锁变量
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;    // 定义并初始化条件变量

struct product
{
    int proId;      // 产品编号
    struct product *next;   // 指向下一个产品的指针   
};

struct product *head = NULL;    // 定义头指针，默认初始化为NULL

int curNum = 0;     // 当前产品数量，默认初始化为0

// 消费者函数
void* consumer(void *arg)
{
    while(1)
    {
        pthread_mutex_lock(&mutex); // 如果消费者线程成功加锁，则生产者线程就会阻塞，如果有多个消费者线程，则其他消费者线程会阻塞
        while(curNum == 0) // 循环等待条件变量，如果缓冲区为空，则消费者线程就会执行下面的wait函数进入阻塞，即不满足条件变量
        {
            pthread_cond_wait(&cond, &mutex);
        
        }
        // 执行到这里说明满足条件变量，消费项...
        struct product *pro = head;
        head = head->next;
        printf("消费者消费了一个产品，产品编号为：%d， 当前产品剩余数量为：%d\n", pro->proId, --curNum);
        free(pro);
        pthread_mutex_unlock(&mutex);
        sleep(rand()%3); // 模拟消费者消费商品需要的时间
    }
    return NULL;
}

// 生产者函数
void* producer(void *arg)
{
    while(1)
    {
        pthread_mutex_lock(&mutex); // 如果生产者线程成功加锁，则消费者线程就会阻塞，如果有多个生产者线程，则其他生产者线程会阻塞
        struct product *pro = (struct product*)malloc(sizeof(struct product));
        pro->proId = rand()%1000 + 1;
        curNum++;
        printf("生产者生产了一个产品，产品编号为：%d， 当前产品剩余数量为：%d\n", pro->proId, curNum);
        /*
        产品是用链表存储的，生产者不用考虑产品的数量，只要生产产品就将产品放到链表中，如果要限制产品的数量，可以定义个宏，当链表中的产品数量达到宏定义的数量时，生产者就不再生产产品，直到消费者消费了产品后，生产者才继续生产产品
        */
        pro->next = head;
        head = pro;
        pthread_mutex_unlock(&mutex);
        pthread_cond_signal(&cond); // 发送条件变量信号，唤醒因为条件变量不满足而阻塞的一个消费者线程，如果有多个消费者线程因为条件变量不满足而阻塞，唤醒哪个消费者线程有调度策略决定
        // 模拟生产者生产商品需要的时间
        sleep(rand()%5);
    }
    return NULL;
}

int main()
{
    pthread_t prod1, prod2, cons1, cons2, cons3;

    // 创建生产者和消费者线程
    pthread_create(&prod1, NULL, producer, NULL);
    pthread_create(&prod2, NULL, producer, NULL);
    pthread_create(&cons1, NULL, consumer, NULL);
    pthread_create(&cons2, NULL, consumer, NULL);
    pthread_create(&cons3, NULL, consumer, NULL);

    // 等待线程结束
    pthread_join(prod1, NULL);
    pthread_join(prod2, NULL);
    pthread_join(cons1, NULL);
    pthread_join(cons2, NULL);
    pthread_join(cons3, NULL);

    // 销毁互斥锁和条件变量
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    return 0;
}
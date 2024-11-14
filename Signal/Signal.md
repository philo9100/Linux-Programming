# 信号

## 7.1 信号及信号来源

```bash
man 7 signal # 查看信号的手册页，手册页中包含了信号的详细信息，包括信号的编号、名称、说明、默认动作等。发送信号的函数
man -k signal | less # 查看所有关于信号的手册页
man -k sigset | less # 查看信号集相关的API
man 3 sigsetops # POSIX signal set operations
ps -p 1 -o pid,ppid,cmd,blocked,pending # 查看进程1的信号屏蔽字和未决信号集
strace -p <pid> # 使用strace命令来跟踪进程的系统调用和信号处理情况。
```bash
less /usr/include/x86_64-linux-gnu/bits/types/__sigset_t.h # 查看信号集的结构体定义
```

```c
#define _SIGSET_NWORDS (1024 / (8 * sizeof (unsigned long int)))
typedef struct
{
  unsigned long int __val[_SIGSET_NWORDS];
} __sigset_t;
```

> 通常产生信号的情况有：
>
> - 用户在终端输入某些按键组合时，终端驱动程序会通知内核产生一个信号，之后内核将该信号发送到相应进程。
> - 当硬件检测到异常时，内核会产生信号并发送信号到正在运行的进程。
> - 满足某些软件条件时，内核也会产生信号并发送到相应进程。
> - 用户通过命令kill向指定进程发送信号。
>
> Linux系统中的信号可能会处于一下几个状态：
>
> - 发送状态：当发生了产生信号的情况时，信号会有一个短暂的发送状态。
> - 阻塞状态：由于某些原因，发送的信号没法被传递，此时信号处于阻塞状态。
> - 未决状态：发送的信号被阻塞，无法到达进程，内核就会将该信号的状态设置为未决状态。
> - 递达状态：信号发送后没有阻塞，信号就会被成功递达到进程，此时信号处于递达状态。
> - 处理状态：信号被递达后会被立刻处理，此时信号处于处理状态。
>
> 信号的处理方式有三种：
>
> - 忽略信号：大多数信号都可以被忽略，但9号信号SIGKILL和19号信号SIGSTOP是超级用户杀死进程的信号，不能被忽略。
> - 捕捉信号：对信号做捕获处理时，进程通常需要先为该信号设置信号响应函数，这是一个回调函数，当信号被递达时，内核会为该进程调用并执行对应的信号响应函数。9号信号SIGKILL和19号信号SIGSTOP不能被捕获。
> - 执行默认操作：系统为每个信号设置了一些默认动作。当信号递达而进程由未设置信号响应函数时，系统会对进程执行信号的默认动作。9号信号SIGKILL和19号信号SIGSTOP的默认动作是终止进程。
>
> 信号的默认动作有5个：
>
> - Term：终止进程
> - Ign：忽略信号
> - Core：终止进程并产生core文件
> - Stop：停止进程
> - Cont：继续运行停止的进程

|信号编号|信号名称|说明|默认动作|
|:---:|:---:|:---:|:---:|
|1|SIGHUP|终端挂起或者控制进程终止|Term|
|2|SIGINT|键盘中断（如：Ctrl+C）|Term|
|3|SIGQUIT|键盘的退出键被按下|Term|
|4|SIGILL|检测到进程执行非法指令|Core|
|5|SIGTRAP|跟踪/断点指令或其他trap指令|Core|
|6|SIGABRT|调用abort函数生成的信号|Core|
|7|SIGBUS|总线错误（内存访问错误，非法访问内存地址）|Core|
|8|SIGFPE|浮点异常|Core|
|9|SIGKILL|用户发出的终止进程信号|Term|
|10|SIGUSR1|用户自定义信号1|Term|
|11|SIGSEGV|无效的内存引用（访问无效内存）|Core|
|12|SIGUSR2|用户自定义信号2|Term|
|13|SIGPIPE|管道破裂：向一个没有读进程的管道写数据|Term|
|14|SIGALRM|时钟定时信号|Term|
|15|SIGTERM|进程终止信号|Term|
|16|SIGSTKFLT|协处理器执行浮点运算，出现栈错误|Term|
|17|SIGCHLD|子进程结束时，父进程会收到这个信号|Ign|
|18|SIGCONT|进程继续（曾被停止的进程）|Cont|
|19|SIGSTOP|停止进程的执行|Stop|
|20|SIGTSTP|键盘上按下停止键（Ctrl+Z），停止终端交互进程的运行|Stop|
|21|SIGTTIN|后台进程读入终端控制台|Stop|
|22|SIGTTOU|后台进程写入终端控制台|Stop|
|23|SIGURG|socket发送紧急情况|Ign|
|24|SIGXCPU|超过CPU时间资源限制|Term|
|25|SIGXFSZ|超过文件大小资源限制|Term|
|26|SIGVTALRM|虚拟时钟信号|Term|
|27|SIGPROF|不仅包括该进程占用CPU的时间，还包括执行系统调用的时间|Term|
|28|SIGWINCH|窗口大小改变|Ign|
|29|SIGIO|异步I/O事件|Term|
|30|SIGPWR|电源故障|Term|
|31|SIGSYS|系统调用错误，无效的系统调用|Core|

## 7.2 信号的产生

### `kill()`函数
1
> `kill()`函数用于向指定进程发送信号，但是否杀死进程取决于所发送信号的默认动作。`kill()`函数存在于系统函数库`signal.h`中，其函数原型为：
>
> ```c
> int kill(pid_t pid, int sig);
> ```
>
> - `pid`：表示接收信号的进程的pid，它的取值有以下几种情况，不同的取值会影响`kill()`函数作用的进程：
>   - `pid` > 0：发送信号sig给进程号为pid的进程。
>   - `pid` = 0：发送信号sig给当前进程所属的进程组中的所有进程。
>   - `pid` = -1：发送信号sig给除1号进程（init进程）与当前进程之外的所有进程。
>   - `pid` < -1：发送信号sig给属于进程组pid的所有进程。
>
> - `sig`：要发送的信号编号。  
> 当其设置为`0`时，`kill()`函数不发送信号，但会执行错误检查，这样可以检测进程是否存在，判断用户进程是否有权限向另外一个进程发送信号。
>
> - `返回值`：成功返回0，失败返回-1，错误代码存于`errno`中。
>   - 若返回值为0，表示当前进程有权限向另外一个存在的进程发送信号。
>   - 若返回值为-1，且errno为ESRCH，表明另外一个指定接收信号的进程不存在；否则表示当前进程没有权限向另外一个进程发送信号。
>
> `kill()`函数发送信号的对象范围取决于调用`kill()`函数的进程的权限，只有`root`用户有权发送信号给任一进程，普通用户进程只能向属于同一进程组或同一用户的进程发送信号。

#### 程序示例：`kill()`函数

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>

int main()
{
    pid_t pid;

    pid = fork();

    if(pid == -1)
    {
        perror("Fork Failed");
        exit(EXIT_FAILURE);
    }
    else if(pid == 0) // 子进程
    {
        sleep(3); // 子进程正在执行sleep函数（即处于睡眠状态）时，父进程发送了SIGSTOP信号，那么子进程将被停止。如果父进程随后发送了SIGCONT信号，那么子进程将从停止状态恢复，继续执行之前的工作。如果子进程在睡眠状态时接收到SIGKILL信号，那么子进程将被立即终止。
        printf("This is child process, pid = %d, ppid = %d\n", getpid(), getppid());
        sleep(3);
        kill(getppid(), SIGKILL); // 在向父进程发送SIGKILL信号后，父进程将被立即终止，子进程将被init进程接管，子进程的ppid将被设置为1。通常情况下，父进程会拥有更高的权限，并且通常会拥有对子进程的控制权。因此，子进程发送SIGKILL信号给父进程可能会破坏系统的一致性。
        sleep(5);
        printf("This is child process, pid = %d, ppid = %d\n", getpid(), getppid());
    }
    else if(pid > 0) // 父进程
    {
        kill(pid, SIGSTOP); // 向子进程发送SIGSTOP信号,使子进程暂停运行，即使子进程处于睡眠状态，也能处理父进程发送的信号。当一个进程接收到信号时，无论它当前处于何种状态（运行、睡眠、停止等），操作系统都会将该信号传递给进程。如果进程已经为该信号注册了处理函数，那么处理函数将被调用。如果没有注册处理函数，那么将执行该信号的默认行为。有一些系统调用（例如I/O操作）可能会导致子进程被阻塞，这时候子进程可能无法立即响应信号。在这种情况下，该信号会被挂起，并在系统调用返回后才会被递送给子进程。需要注意的是，SIGKILL和SIGSTOP是不可捕获或忽略的，它们会立即终止或停止目标进程，而不管其当前状态如何。
        
        printf("This is parent process, pid = %d\nChild process has already been stopped\n", getpid());

        kill(pid, SIGCONT); // 向子进程发送SIGCONT信号,使子进程继续运行
        printf("Child process has already been continued\n");
        // 等待子进程结束，之后处理子进程发送的SIGKILL信号
        printf("Child process has already finished\n I will be killed\n");
        wait(NULL); // 收到SIGKILL信号后，父进程将被立即终止，子进程将被init进程接管，子进程的ppid将被设置为1。
    }
    return 0;
}
```

### `raise()`函数

> `raise()`函数用于向当前进程发送信号，该函数存在于函数库`signal.h`中，其函数原型为：
>
> ```c
> int raise(int sig);
> ```
>
> - `sig`：要发送的信号编号，其取值范围为1~31，若sig的取值不在此范围内，函数将返回-1并将errno设置为EINVAL。
> - `返回值`：成功返回0，失败返回-1，错误代码存于errno中。
>
> `raise()`函数向当前进程发送信号，其作用等同于`kill(getpid(), sig)`。  
> `raise()`函数只能向当前进程发送信号，不能向其他进程发送信号。
>

#### 程序示例：`raise()`函数

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>


int main()
{
    pid_t pid;
    pid = fork();

    if(pid == -1)
    {
        perror("Fork Failed");
        exit(EXIT_FAILURE);
    }
    else if(pid == 0) // 子进程
    {
        printf("This is child process, pid = %d, I am going to stop myself\n", getpid());
        raise(SIGSTOP); // 子进程暂停自己
        sleep(1);
        while(1)
        {
            printf("This is child process, pid = %d, I am alive!\n", getpid());
            sleep(1);
        }
    }
    else if(pid > 0) // 父进程
    {
        sleep(2);
        printf("This is parent process, pid = %d, I launch child process\n", getpid());
        kill(pid, SIGCONT); // 父进程向子进程发送SIGCONT信号，使子进程继续运行
        sleep(5);
        kill(pid, SIGKILL); // 父进程向子进程发送SIGKILL信号，使子进程终止
        wait(NULL); // 等待子进程结束
    }
    return 0;
}
```

### `abort()`函数

> `abort()`函数用于向当前进程发送SIGABRT信号，该函数存在于函数库`stdlib.h`中，其函数原型为：
>
> ```c
> void abort(void);
> ```
>
> `abort()`函数向当前进程发送`SIGABRT`信号，其作用等同于`raise(SIGABRT)`。  
> `abort()`函数只能向当前进程发送信号，不能向其他进程发送信号。
> `abort()`函数会使当前进程终止，其作用等同于调用`exit(EXIT_FAILURE)`。

#### 程序示例：`abort()`函数

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>

int main()
{
    pid_t pid;
    pid = fork();

    if(pid == -1)
    {
        perror("Fork Failed");
        exit(EXIT_FAILURE);
    }
    else if(pid == 0) // 子进程
    {
        printf("This is child process, pid = %d, I am going to stop myself\n", getpid());
        abort(); // 子进程终止自己，并生成core文件
        /*
        当子进程使用`abort()`系统调用来终止自己时，它会生成一个`SIGABRT`信号，这通常导致进程异常终止。一旦这发生，子进程将进入“僵尸”（zombie）状态，直到其父进程读取了子进程的结束状态。
        僵尸状态是一种特殊的状态，进程实际上已经停止执行，不会再进行任何计算，但是它的进程控制块（PCB）和退出状态仍然保存在系统中，以便父进程可以通过`wait()`或`waitpid()`系统调用来检索。
        如果父进程在子进程调用`abort()`后正在执行`sleep()`，它将不会立即意识到子进程的终止。一旦父进程的`sleep()`调用完成，并且父进程执行了相关的等待（wait）操作，它就能读取子进程的结束状态，清理僵尸进程，并释放相关资源。

        即使父进程在子进程结束后延迟了一段时间才检查子进程的状态，操作系统也会保留子进程的结束状态和必要的信息，直到父进程准备好获取它。如果父进程没有及时调用`wait()`或`waitpid()`，子进程的进程控制块将保持在僵尸状态，这可能会导致资源泄漏。
        */
    }
    else if(pid > 0) // 父进程
    {
        sleep(2);
        printf("This is parent process, pid = %d, I launch child process\n", getpid());
        sleep(5);
        kill(pid, SIGKILL); // 父进程向子进程发送SIGKILL信号，使子进程终止
        wait(NULL); // 等待子进程结束，读取子进程的结束状态，清理僵尸进程，并释放相关资源
    }
    return 0;
}
```

### `pause()`函数

pause() causes the calling process (or thread) to sleep until a signal is delivered that either terminates the process or causes the invocation of a signal-catching function.

> `pause()`函数用于使当前进程暂停运行，直到接收到一个信号，该函数存在于函数库`unistd.h`中，其函数原型为：
>
> ```c
> int pause(void);
> ```
>
> `pause()`函数使当前进程暂停运行处于阻塞状态，直到接收到一个信号。
>
> - 若接收到一个信号，且该信号的默认动作是终止进程，则`pause()`函数不会返回，进程将被终止。
> - 若接收到一个信号，且该信号的默认动作是忽略信号，则`pause()`函数不会返回，进程将一直处于阻塞状态。
> - 若接收到一个信号，且该信号的默认动作是捕获信号，在执行完信号处理函数后，则`pause()`函数返回-1，且errno为EINTR。
>
> `pause()`函数只能使当前进程暂停运行，不能使其他进程暂停运行。  
> `pause()`函数返回值为-1，且errno为EINTR，表示当前进程接收到一个信号。

### `alarm()`函数

 alarm() arranges for a SIGALRM signal to be delivered to the calling process in seconds seconds.  
 If seconds is zero, any pending alarm is canceled.  
 In any event any previously set alarm() is canceled.

> `alarm()`函数用于设置一个定时器，当定时器超时时，向调用该函数的当前进程发送SIGALRM信号，该函数存在于函数库`unistd.h`中，其函数原型为：
>
> ```c
> unsigned int alarm(unsigned int seconds);
> ```
>
> - `seconds`：定时器超时时间，单位为秒，其取值范围为0~`UINT_MAX`，若`seconds`的取值不在此范围内，函数将返回-1并将errno设置为EINVAL。
> - `返回值`：成功返回0或上一次定时器剩余的时间，失败返回-1，错误代码存于errno中。  
>
> 计时结束后，内核会向调用`alarm()`函数的进程发送`SIGALRM`信号，如果进程没有捕获该信号，那么进程将被终止。如果进程捕获了该信号，那么进程将执行信号处理函数，然后继续执行。

#### 程序示例：`alarm()`函数

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
    int i;
    alarm(2); // 设置定时器，1秒后超时
    for(i = 0; i < 10; i++)
    {
        printf("This is parent process, pid = %d, i = %d\n", getpid(), i);
        sleep(1);
    }
    return 0;
}
```

### `setitimer()`函数

 These system calls provide access to interval timers, that is, timers that initially expire at some point in the future, and (optionally) at regular intervals after that.  When a timer expires, a signal is generated for the calling process, and the timer is reset to the specified interval (if the interval is nonzero).

> `setitimer()`函数用于设置一个间隔定时器，相比于`alarm()`，它精确到微秒，并且可实现周期定时，该函数存在于函数库`sys/time.h`中，其函数原型为：
>
> ```c
> int setitimer(int which, const struct itimerval *new_value, struct itimerval *old_value);
> ```
>
> - `which`：定时器类型，其取值有以下三种：
>   - `ITIMER_REAL`：以系统真实的时间来计算，它发送`SIGALRM`信号。
>   - `ITIMER_VIRTUAL`：以该进程在用户态下花费的时间来计算，它发送`SIGVTALRM`信号。
>   - `ITIMER_PROF`：以该进程在用户态下和内核态下所费的时间来计算，它发送`SIGPROF`信号。
> - `new_value`：新的定时器设置，其类型为`struct itimerval`，其结构体定义如下：
>  
> ```c
> struct itimerval
> {
>     struct timeval it_interval; // 定时器超时后，重新计时的时间
>     struct timeval it_value; // 定时器超时时间
> };
> 
> struct timeval
> {
>     time_t tv_sec; // 秒
>     suseconds_t tv_usec; // 微秒
> };
> ```
>
> - `old_value`：旧的定时器设置
> - `返回值`：成功返回0，失败返回-1，错误代码存于errno中。

#### 程序示例：`setitimer()`函数

```c
// 该程序实现周期定时
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>

unsigned int my_cycle_alarm(unsigned int sec, unsigned int interval_sec)
{
    struct itimerval new_value, old_value;
    int ret;
    new_value.it_interval.tv_sec = interval_sec;    // 定时器间隔秒    
    new_value.it_interval.tv_usec = 0;  // 微秒 
    new_value.it_value.tv_sec = sec;    // 定时器秒
    new_value.it_value.tv_usec = 0; // 微秒
    ret = setitimer(ITIMER_REAL, &new_value, &old_value);
    if(ret == -1)
    {
        perror("Setitimer Failed");
        exit(EXIT_FAILURE);
    }
    return old_value.it_value.tv_sec;
}

// 该程序实现单次定时
void alarm_handler(int signo)
{
    printf("Caught the SIGALARM signal\n");
}

int main()
{
    int ret;
    struct sigaction sig_act;
    sig_act.sa_handler = alarm_handler;
    sigemptyset(&sig_act.sa_mask);
    sig_act.sa_flags = 0;

    if(ret = sigaction(SIGALRM, &sig_act, NULL) == -1)
    {
        perror("Sigaction Failed");
        exit(EXIT_FAILURE);
    }

    while(1)
    {
        my_cycle_alarm(3, 1); // 设置定时器，3秒后超时，之后每1秒超时一次
        
        pause();    // 暂停进程，直到接收到一个信号
    }

}
```

## 7.3 信号的阻塞

### 信号集设定函数

> 信号集设定函数用于设置信号集，信号集是一个位向量，每个信号对应一个位，信号集设定函数存在于函数库`signal.h`中，其函数原型为：
>
> ```c
> int sigemptyset(sigset_t *set);
> int sigfillset(sigset_t *set);
> int sigaddset(sigset_t *set, int signum);
> int sigdelset(sigset_t *set, int signum);
> int sigismember(const sigset_t *set, int signum);
> ```
>
> - `set`：信号集，其类型为`sigset_t`，其结构体定义如下：
>
> ```c
> typedef struct
> {
>     unsigned long int __val[(1024 / (8 * sizeof(unsigned long int)))];
> } sigset_t;
> ```
>
> - `signum`：信号编号，其取值范围为1~31，若`signum`的取值不在此范围内，函数将返回-1并将errno设置为EINVAL。
>
> - `sigemptyset()`：将信号集清空，即将所有位设置为0。
> - `sigfillset()`：将信号集填满，即将所有位设置为1。
> - `sigaddset()`：将信号集中的某一位设置为1。
> - `sigdelset()`：将信号集中的某一位设置为0。
> - `sigismember()`：判断信号集中的某一位是否为1，若为1返回1，否则返回0。

#### 程序示例：信号集设定函数

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int main()
{
    sigset_t set;   // 信号集
    int ret;
    ret = sigemptyset(&set); // 将信号集清空，即将所有位设置为0
    if(ret == -1)
    {
        perror("Sigemptyset Failed");
        exit(EXIT_FAILURE);
    }
    ret = sigaddset(&set, SIGINT); // 将信号集中的某一位设置为1
    if(ret == -1)
    {
        perror("Sigaddset Failed");
        exit(EXIT_FAILURE);
    }
    ret = sigismember(&set, SIGINT); // 判断信号集中的某一位是否为1，若为1返回1，否则返回0
    if(ret == -1)
    {
        perror("Sigismember Failed");
        exit(EXIT_FAILURE);
    }
    printf("SIGINT is %s in set\n", ret == 1 ? "true" : "false");
    ret = sigismember(&set, SIGKILL);
    if(ret == -1)
    {
        perror("Sigismember Failed");
        exit(EXIT_FAILURE);
    }
    printf("SIGKILL is %s in set\n", ret == 1 ? "true" : "false");
    return 0;
}
```

### `sigprocmask()`函数

> `sigprocmask()`函数用于设置当前进程的信号屏蔽字，信号屏蔽字是一个位向量，每个信号对应一个位，信号屏蔽字中的某一位为1表示该信号被屏蔽，该函数存在于函数库`signal.h`中，其函数原型为：
>
> ```c
> int sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
> ```
>
> - `how`：信号屏蔽字的设置方式，其取值有以下三种：
>   - `SIG_BLOCK`：将信号集中的信号添加到当前进程的信号屏蔽字中。
>   - `SIG_UNBLOCK`：将信号集中的信号从当前进程的信号屏蔽字中删除。
>   - `SIG_SETMASK`：将信号集中的信号设置为当前进程的信号屏蔽字。
> - `set`：信号集，其类型为`sigset_t`。
> - `oldset`：旧的信号屏蔽字，其类型为`sigset_t`。
> - `返回值`：成功返回0，失败返回-1，错误代码存于errno中。

### `sigpending()`函数

> `sigpending()`函数用于获取当前进程的未决信号集，未决信号集是一个位向量，每个信号对应一个位，未决信号集中的某一位为1表示该信号处于未决状态，该函数存在于函数库`signal.h`中，其函数原型为：
>
> ```c
> int sigpending(sigset_t *set);
> ```
>
> - `set`：未决信号集，其类型为`sigset_t`。
> - `返回值`：成功返回0，失败返回-1，错误代码存于errno中。

#### 信号阻塞程序示例

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

// 信号处理函数
void signal_handler(int signo)
{
    printf("Caught the SIGINT signal\n");
}

int main()
{
    sigset_t new_set, old_set, pending_set;

    sigemptyset(&new_set);  // 将自定义信号集清空，即将所有位设置为0
    sigaddset(&new_set, SIGINT);    // 将 SIGINT 信号添加到信号集中

    // 判断 SIGINT 信号是否在自定义信号集中，若在返回1，否则返回0
    if(sigismember(&new_set, SIGINT) == 1)  
    {
        printf("SIGINT is in new_set\n");
    }
    else
    {
        printf("SIGINT is not in new_set\n");
    }

    // 注册信号处理函数
    if(signal(SIGINT, signal_handler) == SIG_ERR) 
    {
        perror("Signal Failed");
        exit(EXIT_FAILURE);
    }

    // 设置当前进程的屏蔽信号集，并将旧的屏蔽信号集保存到 old_set 中
    // 用户发送了 SIGINT（通常是通过按下 Ctrl+C），信号也不会被递送到进程中。
    if(sigprocmask(SIG_BLOCK, &new_set, &old_set) == -1)
    { 
        perror("Sigprocmask Failed");
        exit(EXIT_FAILURE);
    }

    printf("SIGINT is blocked. Try pressing Ctrl+C. You will see that the signal is pending.\n");
    sleep(10); // 给出足够时间让用户有机会发送 SIGINT 信号，但是信号被阻塞了

    // 获取当前进程的未决信号集，即当前进程收到但是由于被阻塞等原因未被处理的信号的信号集
    if(sigpending(&pending_set) == -1)
    {
        perror("Sigpending Failed");
        exit(EXIT_FAILURE);
    }

    // 判断 SIGINT 信号是否在未决信号集中，若在返回1，否则返回0
    if(sigismember(&pending_set, SIGINT) == 1)
    {
        printf("SIGINT is pending\n");
    }
    else
    {
        printf("SIGINT is not pending\n");
    }

    // 恢复到旧的信号屏蔽字，解除对 SIGINT 信号的阻塞
    if(sigprocmask(SIG_SETMASK, &old_set, NULL) == -1)
    {
        perror("Sigprocmask Failed");
        exit(EXIT_FAILURE);
    }

    // 如果在阻塞期间有 SIGINT 信号发送到进程，此信号将在解除阻塞后立即被处理。
    printf("SIGINT is unblocked.\n");

    // 程序再次睡眠一秒钟，以确保信号处理函数有机会执行。
    sleep(1); 

    return 0;
}
```

## 7.4 信号的捕获

### `signal()`函数

> `signal()`函数用于为指定信号设置信号响应函数，该函数存在于函数库`signal.h`中，其函数原型为：
>
> ```c
> void (*signal(int signum, void (*handler)(int)))(int);
> ```
>
> - `signum`：信号编号
> - `handler`：信号响应函数，其类型为`void (*)(int)`，即指向函数的指针，该函数的参数为`int`类型，返回值为`void`类型。
>   - 若`handler`为`SIG_IGN`，则表示忽略信号。
>   - 若`handler`为`SIG_DFL`，则表示执行信号的默认动作。
> - `返回值`：成功返回旧的信号响应函数，失败返回`SIG_ERR`。

#### 程序示例：`signal()`函数

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

// 信号处理函数
void signal_handler(int signo)
{
    printf("Caught the SIGINT signal\n");
}

int main()
{
    // 注册信号处理函数
    if(signal(SIGINT, signal_handler) == SIG_ERR)
    {
        perror("Signal Failed");
        exit(EXIT_FAILURE);
    }

    while(1)
    {
        printf("Try pressing Ctrl+C\n");
        printf("This is parent process, pid = %d\n", getpid());
        sleep(1);
    }
    /*
    在bash窗口循环输出信息，直到按下Ctrl+C，执行信号处理函数，然后继续循环输出信息
    可以按下Ctrl+Z，暂停程序，通过jobs命令查看当前被挂起的进程的jobid，然后在bash窗口输入fg %1 (bg %jobid 在后台运行)，继续执行程序  
    在程序暂停时，使用kill命令杀死进程，会看到暂停中的进程为被杀死，但是在程序继续运行时，会看到程序被杀死      
    */

    return 0;
}
```

### `sigaction()`函数

> `sigaction()`函数用于为指定信号设置信号响应函数，该函数存在于函数库`signal.h`中，其函数原型为：
>
> ```c
> int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);
> ```
>
> - `signum`：信号编号
> - `act`：新的信号响应函数，其类型为`struct sigaction`，其结构体定义如下：
>
> ```c
> struct sigaction
> {
>     void (*sa_handler)(int); // 用于指定信号处理函数的地址，可以是一个函数指针，也可以是SIG_IGN或SIG_DFL。
>     sigset_t sa_mask; // 指定在信号处理函数执行期间需要阻塞的信号集合。
>     int sa_flags; // 用于指定信号处理的一些属性标志，比如SA_RESTART、SA_NODEFER等。
>     void (*sa_sigaction)(int, siginfo_t *, void *); // 用于指定信号的处理函数，并提供了更多的关于信号的信息。与sa_handler是互斥的，只能选择其一进行设置。
>     void (*sa_restorer)(void); // 
> // `sa_handler`为`SIG_IGN`，则表示忽略信号。
> // `sa_handler`为`SIG_DFL`，则表示执行信号的默认动作。
> 
> // 在sigaction结构体中，结构体成员sa_flags用于指定对信号处理程序的行为。它可以包含以下标志的组合：
> // SA_NOCLDSTOP：当子进程停止时不会给父进程发送SIGCHLD信号。
> // SA_NOCLDWAIT：父进程不会在子进程退出时收到SIGCHLD信号。
> // SA_NODEFER：在信号处理程序执行期间，不会自动阻塞当前正在处理的信号。
> // SA_RESETHAND：在信号处理程序执行之后，将信号的处理程序重置为SIG_DFL。
> // SA_RESTART：如果系统调用被信号中断，则自动重启系统调用。
> };
>
> struct siginfo_t
> {
>     int si_signo; // 信号编号
>     int si_errno; // 如果信号是由系统调用产生的，则该字段保存了系统调用的错误码。
>     int si_code; // 信号产生的原因，比如SIGILL信号产生的原因可能是非法指令、SIGFPE信号产生的原因可能是浮点异常等。
>     pid_t si_pid; // 发送信号的进程的pid
>     uid_t si_uid; // 发送信号的进程的uid
>     int si_status; // 如果信号是由子进程产生的，则该字段保存了子进程的退出状态。
>     clock_t si_utime; // 用户态运行时间
>     clock_t si_stime; // 内核态运行时间
>     sigval_t si_value; // 信号附带的数据
>     int si_int; // 信号附带的数据
>     void *si_ptr; // 信号附带的数据
>     void *si_addr; // 产生信号的内存地址
>     int si_band; // 产生信号的I/O事件
>     int si_fd; // 产生信号的文件描述符
> };
> ```
>
> - `oldact`：旧的信号响应函数，其类型为`struct sigaction`。
> - `返回值`：成功返回0，失败返回-1，错误代码存于errno中。

#### 程序示例：`sigaction()`函数

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>


// 信号处理函数
void signal_handler(int signo)
{
    printf("Caught the SIGINT signal\n");
    sleep(5);   // 模拟信号处理函数执行
}

int main()
{
    struct sigaction act, old_act;
    act.sa_handler = signal_handler;    // 用于指定信号处理函数的地址，可以是一个函数指针，也可以是SIG_IGN或SIG_DFL。
    sigemptyset(&act.sa_mask);  // 指定在信号处理函数执行期间需要阻塞的信号集合。
    sigaddset(&act.sa_mask, SIGINT);    // 将 SIGINT 信号添加到信号集中
    act.sa_flags = SA_RESETHAND;   // 执行完信号处理函数后，将信号的处理程序重置为SIG_DFL。
    /*
    第一次按下ctrl+c，执行信号处理函数，然后将信号处理函数重置为默认行为
    第二次按下ctrl+c，执行默认行为，即终止进程
    */
    sigaction(SIGINT, &act, &old_act);  // 注册信号处理函数

    while(1)
    {
        printf("Try pressing Ctrl+C\n");
        printf("This is parent process, pid = %d\n", getpid());
        sleep(1);
    }

}
```

#### 自实现`sleep()`函数

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

void sig_alrm(int signo)
{
    // 什么都不做，只是为了唤醒pause
}

unsigned int my_sleep(unsigned int sec)
{
    struct sigaction new_act, old_act;
    new_act.sa_handler = sig_alrm;
    sigemptyset(&new_act.sa_mask);
    new_act.sa_flags = 0;
    sigaction(SIGALRM, &new_act, &old_act); // 注册信号处理函数，捕获SIGALRM信号.执行信号处理函数，如果没有捕获SIGALRM信号将执行默认行为，那么进程将被终止。
    alarm(sec); // 设置定时器，sec秒后超时，向当前进程发送SIGALRM信号
    sigaction(SIGALRM, &old_act, NULL); // old_act中没有绑定SIGALRM的信号处理函数，即恢复SIGALRM信号的默认行为
    pause(); // 暂停进程，直到接收到一个信号
    return alarm(0); // 取消定时器，返回定时器剩余的时间
}

int main()
{
    while(1)
    {
        my_sleep(3);
        printf("three seconds passed\n");
    }
    return 0;
}
```

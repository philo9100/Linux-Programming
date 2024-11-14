# Linux进程管理

## 进程控制

### `fork()`函数

> `fork()`函数的功能用于创建子进程，调用`fork()`函数的进程叫父进程，`fork()`函数被调用一次，但返回两次，两次返回的区别是子进程返回0，父进程返回子进程的进程号。该函数存在于系统函数库`unistd.h`中，其函数原型为：
>
> ```c
> pid_t fork(void);
> ```
>
> - 返回值：子进程返回0，父进程返回子进程的进程号，出错返回-1
>
> `fork()`函数返回两次的原因：  
> `fork()`函数在被调用后，原程序会被复制，那么就会产生两个`fork()`函数，子进程创建成功后，父进程中的`fork()`函数会返回子进程的`pid`，子进程中的`fork()`函数会返回`0`；若子进程创建失败，原程序不会被复制，父进程的`fork()`函数返回`-1`
>
> 在调用fork()函数创建子进程后，父进程和子进程会拥有各自独立的虚拟地址空间。虽然刚开始时父进程和子进程的虚拟地址空间内容是一样的，子进程获得父进程的代码段、数据段、堆栈、页表等副本，父子进程中的虚拟地址相同，虚拟地址映射的物理地址也相同。若进程只需要进行数据的访问，则通过相同的虚拟地址映射到物理地址即可。若子进程要对数据段、堆栈中的数据进行修改，则系统将待操作的物理内存页进行复制，子进程修改的是复制后的物理内存页，这样就实现了父子进程之间的数据独立。子进程的虚拟地址映射到修改后的物理内存地址，父进程的虚拟地址依然映射到原物理内存地址。

#### `fork()`程序示例

```c
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>


int main()
{
    pid_t pid; // 声明进程标识符

    pid = fork(); // 创建子进程

    if(pid == -1) // 创建失败
    {
        fprintf(stderr, "Failed to fork: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }   
    else if(pid == 0) // 子进程
    {
        printf("This is child process, pid = %d\n", getpid());
    }
    else if(pid > 0)// 父进程
    {
        printf("This is parent process, pid = %d\n", getpid());

    }

    printf("***********finish***********\n");   
    return 0;
}
```

多次执行`fork()`函数会出现的情况是子进程的的`ppid`不等于父进程的`pid`，而等于1，其原因是：这种情况通常发生在父进程在子进程之前结束，子进程被`init`进程（pid为1）接管，子进程的`ppid`将被设置为`1`。这是为了防止产生僵尸进程。当一个进程结束时，它的进程描述符不会立即被释放，而是等待父进程读取它的退出状态。如果父进程在子进程之前结束，那么子进程就没有父进程来读取它的退出状态，这个子进程就会变成僵尸进程。为了防止这种情况，子进程将被`init`进程接管，`init`进程将负责读取它们的退出状态，从而避免僵尸进程的产生。

#### 程序示例：创建多个进程

```c
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

int main()
{
    pid_t pid; // 声明进程标识符
    int i;

    for(i = 0; i < 5; ++i) // 循环创建5个子进程，避免子进程创建子进程要进行判断
    {
        if((pid = fork()) == 0) // 如果当前是子进程执行fork就退出循环 
        {
            break;
        }
    }
    if(pid == -1)
    {
        fprintf(stderr, "Failed to fork: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
    else if(pid > 0)
    {
        sleep(5);
        printf("This is parent process, pid = %d\n", getpid());
    }
    else if(pid == 0)
    {
        sleep(i);
        printf("This is %d child process, pid = %d\n", i+1, getpid());
    }
    return 0;
}
```

### `exec()`函数族

> `exec()`函数族用于执行一个新的程序，它会覆盖原进程的数据段、代码段、堆栈，从新程序的`main()`函数开始执行，原进程的进程号不变，该函数存在于系统函数库`unistd.h`中，其函数原型为：
>
> ```c
> int execl(const char *path, const char *arg, ...);
> int execlp(const char *file, const char *arg, ...);
> int execle(const char *path, const char *arg, ..., char * const envp[]);
> int execv(const char *path, char *const argv[]);
> int execvp(const char *file, char *const argv[]);
> int execvpe(const char *file, char *const argv[], char *const envp[]);
> ```
>
> 函数的第一个参数为file或path，它们的区别在于：当参数是path时，传入的数据为路径名；当参数是file时，传入的数据为可执行文件名，此时系统会在PATH环境变量中查找该文件名对应的文件，如果找到则执行该文件，否则返回-1，如果数据中包含"/"则将其视为路径名。
>
> `exec()`函数族用于在当前进程中执行一个新的程序。它将当前进程的映像替换为新程序的映像，新程序从其`main()`函数开始执行，然后在退出时结束。这意味着，一旦`exec()`被调用并成功，它就不会返回到原来的程序中。

#### 程序示例：exec()函数族

```c
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main()
{
    pid_t pid; // 声明进程标识符
    pid = fork(); // 创建子进程

    if (pid == -1) // 进程创建失败
    {
        perror("Fork Faild");
        exit(EXIT_FAILURE);
    }
    else if(pid > 0) // 父进程
    {
        /*
        在C语言中，wait()函数用于使父进程暂停执行，直到它的一个子进程结束为止。wait()函数的参数是一个指向int类型的指针，用于存储子进程的退出状态。如果你将NULL作为参数传递给wait()函数，那么意味着你不关心子进程的退出状态。你只是想让父进程等待，直到有一个子进程结束。在这种情况下，子进程的退出状态将被丢弃。这是一个常见的用法，因为在许多情况下，父进程并不真正关心子进程的退出状态，它只是需要知道子进程何时结束。*/
        wait(NULL); // 等待子进程结束，不考虑子进程的退出状态
        printf("This is parent process, pid = %d\n", getpid());
        printf("child process has finished\n");
    }
    else if(pid == 0) // 子进程
    {
        char* arguePath = "/home/philo/professional/CSCourse/LinuxPractice/Process/exer1";
        printf("This is child process, pid = %d\n", getpid());
        // execl("/bin/ls", "ls", "-l", arguePath, NULL); // 执行ls -l命令
        /*
        在执行exec()被调用并成功，它就不会返回到原来的程序中，也就是说下面的程序不会再执行，这也是exec函数族存在的意义
        若要执行下面的程序，需要在exec函数族调用之前创建子进程或者注释掉上面的exec函数族调用
        */
        sleep(5);
        
        char *arguePath2[] = {"ls", "-l", arguePath, NULL};
        execv("/bin/ls", arguePath2); // 执行ls -l命令

        perror("Exec Faild"); // 如果执行失败，输出错误信息
        printf("child process pid = %d has finished\n", getpid());
    }

    return 0;
}
```

```c
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main()
{
    // 新程序的路径
    const char *path = "/home/philo/professional/CSCourse/LinuxPractice/Process/exer1";

    // 假设新程序期望接收两个参数
    const char *arg1 = "arg1";
    const char *arg2 = "arg2";

    // 设定新程序的环境变量
    // 当一个新程序被执行时，它可能需要不同的环境变量来满其特定的运行要求。这些环境变量可能包括程序的运行时配置、语言设置、路径信息等。因此，每个新程序都需要自己的环境变量，以确保其能够在适当的环境中正确运行。例如，不同的程序可能需要不同的PATH变量来定位可执行文件、不同的LD_LIBRARY_PATH变量来指定动态链接库的搜索路径、不同的LANG变量来设置语言环境等。因此，每个新程序都需要自己的环境变量来满足其独特的运行要求。
    char* const envp[] = {
        "HOME=/", 
        "PATH=/bin:/usr/bin",
        // ...
        NULL    // 环境变量列表以NULL结尾
    };

    pid_t pid; // 声明进程标识符
    pid = fork(); // 创建子进程

    if (pid == -1) // 进程创建失败
    {
        perror("Fork Faild");
        exit(EXIT_FAILURE);
    }
    else if(pid > 0) // 父进程
    {
        wait(NULL); // 等待子进程结束，不考虑子进程的退出状态
        printf("This is parent process, pid = %d\n", getpid());
        printf("child process has finished\n");
    }
    else if(pid == 0) // 子进程
    {
        printf("This is child process, pid = %d\n", getpid());
        if(execle(path, arg1, arg2, NULL, envp) == -1)
        {
            perror("Exec Faild"); // 如果执行失败，输出错误信息
            exit(EXIT_FAILURE);
        }
    }

    // 如果子进程执行execle()函数成功，那么子进程将不会执行下面的代码
    return 0;
}
```

## 进程退出

### `exit()`函数

> `exit()`函数用于正常终止进程，该函数存在于系统函数库`stdlib.h`中，其函数原型为：
>
> ```c
> void exit(int status);
> ```
>
> - `status`为进程的退出状态，该状态可以被父进程用来判断子进程的退出状态，标准c中定义了两个宏：`EXIT_SUCCESS`和`EXIT_FAILURE`，分别表示进程正常退出和进程异常退出，其值分别为`0`和`1`

#### 两个特殊进程：孤儿进程和僵尸进程

> 在Linux中父子进程是异步运行的，父进程可能会在子进程之前结束，当一个父进程结束，而它的一个或多个子进程还在运行，那么这些子进程就会成为孤儿进程。孤儿进程将被 init 进程（进程ID为1）接管。init 进程是系统启动时创建的第一个进程，它不会结束。当 init 进程接管一个孤儿进程时，它将成为这个孤儿进程的新的父进程。这意味着，孤儿进程不会因为没有父进程而结束，它们可以继续运行。当孤儿进程结束时，init 进程将负责清理和回收资源，防止孤儿进程成为僵尸进程。
>
> 在Linux中，当一个进程结束运行，但其父进程没有调用 wait() 或waitpid() 来读取它的结束状态，这个进程就会成为僵尸进程。尽管这个进程已经结束，但是它在进程表中仍然有一个条目，这就是所谓的僵尸进程。僵尸进程不再需要任何系统资源（除了进程表中的一个条目）并且不能被调度。如果父进程结束，所有的僵尸进程将被 init 进程（进程ID为1）接管，init 进程将定期调用 wait() 来清除僵尸进程。

## 进程同步

### `wait()`函数

> `wait()`函数存在于系统函数库`sys/wait.h`中，其函数原型为：
>
> ```c
> pid_t wait(int *status);
> ```
>
> - `status`为进程的退出状态，该状态可以被父进程用来判断子进程的退出状态，标准c中定义了两个宏：`EXIT_SUCCESS`和`EXIT_FAILURE`，分别表示进程正常退出和进程异常退出，其值分别为`0`和`1`
> - 返回值：成功返回子进程的进程号，失败返回-1
>
> 调用`wait()`函数的进程会被挂起，进入阻塞状态，直到子进程变为僵尸态，`wait()`函数捕获到该子进程的退出信息时才会转为运行态，回收子进程的资源并返回；若没有变为僵尸态的子进程，`wait()`函数会让调用它的进程一直阻塞。若当前进程有多个子进程，`wait()`函数捕获到一个变为僵尸态的子进程的信息就返回并继续执行，若要回收所有子进程的资源，需要多次调用`wait()`函数。

#### 程序示例：wait()函数

```c
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

int main()
{
    int status;
    pid_t pid, w; // 声明进程标识符
    pid = fork();

    if(pid == -1)
    {
        perror("fork error");
        exit(EXIT_FAILURE);
    }
    else if(pid == 0)
    {
        sleep(3);
        printf("Child process : pid = %d\n", getpid());
        exit(EXIT_SUCCESS);
    }
    else if(pid > 0)
    {   
        // 父进程调用wait()函数，阻塞等待子进程结束
        w = wait(&status);
        if(w == -1)
        {
            perror("wait error");
            exit(EXIT_FAILURE);
        }
        /*
        宏函数WIFEXITED(status)用于判断子进程是否正常结束，若正常结束则返回非0值，否则返回0
        宏函数WEXITSTATUS(status)用于获取子进程的退出状态，只有在WIFEXITED(status)返回非0值时才有意义
        */
        if(WIFEXITED(status))
        {
            printf("Child process exited normally, status = %d\n", WEXITSTATUS(status));
        }
        /*
        宏函数WIFSIGNALED(status)用于判断子进程是否异常结束，若异常结束则返回非0值，否则返回0
        */
        else if(WIFSIGNALED(status))
        {
            printf("Child process exited abnormally, signal number = %d\n", WTERMSIG(status));
        }
        printf("Parent process : pid = %d\n", getpid());
    }
    return 0;
}
```

### `waitpid()`函数

> `waitpid()`函数存在于系统函数库`sys/wait.h`中，其函数原型为：
>
> ```c
> pid_t waitpid(pid_t pid, int *status, int options); 
> ```
>
> - `pid`
>   - pid > 0：等待进程号为pid的子进程结束
>   - pid = 0：等待与调用该函数的进程所属同一个进程组的所有子进程结束
>   - pid = -1：等待任意子进程结束，与`wait()`函数功能相同
> - `option`
>   - `WNOHANG`：若pid指定的子进程没有结束，则waitpid()函数返回0，不予以等待
>   - `WUNTRACED`：若子进程进入暂停状态，则立即返回
> - 返回值：成功返回子进程的进程号，失败返回-1。若`option`为`WNOHANG`，则若没有子进程结束则返回0
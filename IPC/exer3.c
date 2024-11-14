#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    int pipefd[2];
    pid_t child1_pid, child2_pid, child3_pid, child4_pid;
    char buffer[20];
    int nbytes;

    // 创建管道
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // 创建第一个子进程，负责向管道写入数据
    if ((child1_pid = fork()) == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if (child1_pid == 0) {
        close(pipefd[0]);  // 关闭读端
        char *msg1 = "Hello from child 1";
        write(pipefd[1], msg1, sizeof(msg1));
        exit(0);
    }

    // 创建第二个子进程，负责向管道写入数据
    if ((child2_pid = fork()) == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if (child2_pid == 0) {
        close(pipefd[0]);  // 关闭读端
        char *msg2 = "Hello from child 2";
        sleep(5);
        write(pipefd[1], msg2, sizeof(msg2));
        exit(0);
    }

    // 创建第三个子进程，负责从管道读取数据
    if ((child3_pid = fork()) == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if (child3_pid == 0) {
        close(pipefd[1]);  // 关闭写端
        nbytes = read(pipefd[0], buffer, sizeof(buffer));
        printf("Child 3 received: %s\n", buffer);
        exit(0);
    }

    // 创建第四个子进程，负责从管道读取数据
    if ((child4_pid = fork()) == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if (child4_pid == 0) {
        close(pipefd[1]);  // 关闭写端
        sleep(5);
        nbytes = read(pipefd[0], buffer, sizeof(buffer));
        printf("Child 4 received: %s\n", buffer);
        exit(0);
    }

    // 父进程关闭管道两端
    close(pipefd[0]);
    close(pipefd[1]);

    return 0;
}

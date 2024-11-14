#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#define BUFFER_SIZE 1024

void sig_handler(int signum){
    alarm(1);
    printf("child done\n");
    exit(EXIT_FAILURE);
}

int main()
{
    int pipefd[2];
    int ret;
    pid_t pid;
    ret = pipe(pipefd);
    if(ret == -1)
    {
        fprintf(stderr, "failed to pipe:%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    pid = fork();

    if (pid > 0) { // parent 
        close(pipefd[0]);
        char buf[BUFFER_SIZE] = {0};
        
        while (1) {
            printf("请输入内容：\n");
            scanf("%s", buf);
            if (buf[0] == '\n') {
                kill(pid, SIGCHLD);
                break;
            }
            write(pipefd[1], buf, BUFFER_SIZE);
        }
        
        waitpid(pid, NULL, 0);

        printf("done\n");
    }
    else if(pid == 0) {
        close(pipefd[1]);
        char buf_read[BUFFER_SIZE] = {0};

        struct sigaction act, oldact;
        act.sa_handler = sig_handler;
        sigemptyset(&act.sa_mask);
        act.sa_flags = 0;
        sigaction(SIGCHLD, &act, &oldact);
        
        while(1) {
            read(pipefd[0], buf_read, BUFFER_SIZE);
            printf("%s\n", buf_read);
        }
    }

    return 0;
}


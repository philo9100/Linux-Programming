#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <signal.h>

#define MAX_MSIZE 1024

struct msgbuf msg;
int msqid;
key_t key;

struct msgbuf{
    long mtype;
    char mtext[MAX_MSIZE];
};


int main() {
    
    int ret;
   

    if((key = ftok("./exam2.c","a")) == -1){
        fprintf(stderr, "failed to ftok: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if((msqid = msgget(key, IPC_CREAT | IPC_EXCL | 0777)) == -1){
        fprintf(stderr, "failed to msgget: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    while(1){
        printf("enter some message:\n");
        fgets(msg.mtext, MAX_MSIZE, stdin);
        msg.mtype = 666;
        
        if ((ret =msgsnd(msqid, &msg, MAX_MSIZE, 0) == -1)) {
            fprintf(stderr, "failed to msgsnd: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        } 
        if (strncmp(msg.mtext, "--exit", 6)) {
            break;
        }
    }
}
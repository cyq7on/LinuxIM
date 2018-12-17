#include <stdio.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

#define BACKLOG 1


int ftArray[MAXCLIENT] = {0};
char *user[MAXCLIENT];
char buf[BUFFSIZE];

void *pthread_service(void *sfd)
{
    int fd = *(int *)sfd;
    while (1)
    {
        int numbytes;
        int i;
        numbytes = recv(fd, buf, BUFFSIZE, 0);
        if (numbytes <= 0)
        {
            for (i = 0; i < MAXCLIENT; i++)
            {
                if (fd == ftArray[i])
                {
                    ftArray[i] = 0;
                }
            }
            printf("numbytes=%d\n", numbytes);
            printf("fd=%d exit\n", fd);
            break;
        }
        /* if (strncmp(buf, "exit", 4) == 0) {

        } */
        printf("receive message from %d,size=%d\n", fd, numbytes);
        
        if(strncmp(buf, "/p", 2) == 0) {
            /* char *spilit;
            char *arr[MAXCLIENT];
            
            int i = 0;
            while (spilit = strtok(buf," ")) {
                arr[i++] = spilit;
            }
            char *msg = arr[--i];
            printf("msg:%s",msg); */

        }else {
            sendMsg(fd, buf, numbytes);
        }
        bzero(buf, BUFFSIZE);
    }
    close(fd);
}

// 发送消息到客户端
int sendMsg(int fd, char *buf, int Size)
{
    int i;
    for (i = 0; i < MAXCLIENT; i++)
    {
        printf("ftArray[%d]=%d\n", i, ftArray[i]);
        if ((ftArray[i] != 0) && (ftArray[i] != fd))
        {
            send(ftArray[i], buf, Size, 0);
            printf("send message to %d\n", ftArray[i]);
        }
    }
    return 0;
}

int main()
{

    int listenfd, connectfd;
    struct sockaddr_in server;
    struct sockaddr_in client;
    int sin_size;
    sin_size = sizeof(struct sockaddr_in);
    int number = 0;
    int fd;

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Create socket error\n");
        exit(1);
    }

    int opt = SO_REUSEADDR;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    bzero(&server, sizeof(server));

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listenfd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    {
        perror("Bind error");
        exit(1);
    }

    if (listen(listenfd, BACKLOG) == -1)
    {
        perror("Listen error\n");
        exit(1);
    }
    printf("Sever are ready!\n");

    while (1)
    {

        if ((fd = accept(listenfd, (struct sockaddr *)&client, &sin_size)) == -1)
        {
            perror("Accept error\n");
            exit(1);
        }

        if (number >= MAXCLIENT)
        {
            printf("Client is full\n");
            close(fd);
        }

        int i;
        for (i = 0; i < MAXCLIENT; i++)
        {
            if (ftArray[i] == 0)
            {
                ftArray[i] = fd;
                break;
            }
        }

        pthread_t tid;
        pthread_create(&tid, NULL, (void *)pthread_service, &fd);

        number = number + 1;
    }

    close(listenfd);
}

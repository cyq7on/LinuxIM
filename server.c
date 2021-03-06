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
void reset(int userId);

void *pthread_service(void *sfd)
{
    int fd = *(int *)sfd;
    Msg *msg = (Msg *)malloc(size);
    char *buffer = (char *)malloc(size);
    while (1)
    {

        int pos = 0;
        int len;
        while (pos < size)
        {
            len = recv(fd, buffer + pos, BUFFSIZE, 0);
            if (len < 0)
            {
                printf("recv error\n");
                pthread_exit(NULL);
            }
            pos += len;
        }

        memcpy(msg, buffer, size);
        msg->userFd = fd;

        printf("receive message from %d,size=%ld\n", fd, strlen(msg->content));
        printf("%s,%s,%d\n", msg->userName, msg->content,msg->type);

        memset(buffer, 0, size);
        memcpy(buffer, msg, size);

        /* switch (msg->type)
        {
        case DEFAULT:
            break;
        case HELP:
            break;
        case ENTER:
            break;
        case EXIT:
            reset(msg->userFd);
            break;
        } */

        // 下线，fd置零
        if(msg->type == EXIT) {
            reset(msg->userFd);
        }

        // 指定了接收人
        if (msg->recvUserId[0])
        {
            for (int i = 0; msg->recvUserId[i]; i++)
            {
                send(msg->recvUserId[i], buffer, size, 0);
                printf("send private message to %d\n", ftArray[i]);
            }
        }
        // 广播，包括发送人
        else if (msg->type == WITHDRAW)
        {
            int i;
            for (i = 0; i < MAXCLIENT; i++)
            {
                if ((ftArray[i] != 0))
                {
                    send(ftArray[i], buffer, size, 0);
                    printf("send withdraw message to %d\n", ftArray[i]);
                }
            }
        }
        // 群发
        else
        {
            sendMsg(fd, buffer, size);
        }

        
        /* if (strncmp(buf, "/p", 2) == 0)
        {
            char *spilit;
            char *arr[MAXCLIENT];
            
            int i = 0;
            while (spilit = strtok(buf," ")) {
                arr[i++] = spilit;
            }
            char *msg = arr[--i];
            printf("msg:%s",msg); 
        }
        else
        {
            sendMsg(fd, buffer, size);
        } */
        // bzero(buf, BUFFSIZE);
        memset(msg, 0, size);
        memset(buffer, 0, size);
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

// 根据id获取索引
int getIndex(int userId)
{
    for (int i = 0; i < MAXCLIENT; i++)
    {
        if (userId == ftArray[i])
        {
            return i;
        }
    }
    return -1;
}

void reset(int userId)
{
    int index = getIndex(userId);
    if (index)
    {
        ftArray[index] = 0;
    }
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

    printf("close fd\n");
    close(listenfd);
}

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include<pthread.h> 
#include "common.h"


char sendbuf[BUFFSIZE];
char recvbuf[BUFFSIZE];
char name[32];
int fd;

void pthread_recv(void *ptr)
{
    while (1)
    {
        // 接收数据
        if ((recv(fd, recvbuf, BUFFSIZE, 0)) == -1)
        {
            printf("recv error\n");
            exit(1);
        }
        printf("%s", recvbuf);
        // recvbuf初始化为0
        memset(recvbuf, 0, sizeof(recvbuf));
    }
}

int main(int argc, char *argv[])
{
    char buf[BUFFSIZE];
    struct hostent *host;
    struct sockaddr_in server;

    if (argc != 2)
    {
        argv[1] = "127.0.0.1";
        printf("Use default ip address:%s\n", argv[1]);
    }

    if ((host = gethostbyname(argv[1])) == NULL)
    {
        printf("gethostbyname error\n");
        exit(1);
    }

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("Create socket error\n");
        exit(1);
    }

    bzero(&server, sizeof(server));

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr = *((struct in_addr *)host->h_addr);
    // 连接
    if (connect(fd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    {
        printf("connect error\n");
        exit(1);
    }

    printf("connect success\n");
    char str[] = "已上线\n";
    printf("请输入用户名：");
    fgets(name, sizeof(name), stdin);
    send(fd, name, (strlen(name) - 1), 0);
    send(fd, str, (strlen(str)), 0);

    pthread_t tid;
    pthread_create(&tid, NULL, pthread_recv, NULL);

    while (1)
    {
        memset(sendbuf, 0, sizeof(sendbuf));
        fgets(sendbuf, sizeof(sendbuf), stdin);
        // 比较前四个字符
        if (strncmp(sendbuf, "exit", 4) == 0)
        {
            memset(sendbuf, 0, sizeof(sendbuf));
            printf("您已下线\n");
            send(fd, sendbuf, (strlen(sendbuf)), 0);
            break;
        }
        send(fd, name, (strlen(name) - 1), 0);
        send(fd, ":", 1, 0);

        send(fd, sendbuf, (strlen(sendbuf)), 0);
    }
    close(fd);
}

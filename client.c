#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <pthread.h>
#include "common.h"
#include <dlfcn.h>

char sendbuf[BUFFSIZE];
char recvbuf[BUFFSIZE];
char name[32];
int fd;

void pthread_recv(void *ptr)
{
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
                exit(1);
            }
            pos += len;
        }

        memcpy(msg, buffer, size);

        // 撤回
        if (msg->type == WITHDRAW)
        {
            if (strncmp(msg->userName, name ,strlen(msg->userName)) == 0)
            {
                printf("您已成功撤回消息\n");
            }
            else
            {
                printf("%s撤回一条消息\n", msg->userName);
            }
        }
        else
        {
            printf("%s-%d:%s\n", msg->userName, msg->userFd, msg->content);
        }
        memset(msg, 0, size);
        memset(buffer, 0, size);

        /*  free(buffer);
        free(msg); */
        // 接收数据
        /* if ((recv(fd, recvbuf, BUFFSIZE, 0)) == -1)
        {
            printf("recv error\n");
            exit(1);
        }
        printf("%s", recvbuf);
        // recvbuf初始化为0
        memset(recvbuf, 0, sizeof(recvbuf)); */
    }
}

// 运行插件
int runPlugin(char *pname)
{
    void (*pfunc)();
    //错误信息字符串
    char *errorMessage;
    char libname[32];
    sprintf(libname, "%s%s%s", "./lib/", pname, ".so");
    void *handle = dlopen(libname, RTLD_LAZY);
    errorMessage = dlerror();
    if (handle == NULL)
    {
        printf("plugin dlopen error:%s\n", errorMessage);
        return 0;
    }
    *(void **)(&pfunc) = dlsym(handle, "process");
    errorMessage = dlerror();
    if (pfunc == NULL)
    {
        printf("plugin method error:%s\n", errorMessage);
        return 0;
    }
    pfunc();
    dlclose(handle);
    return 1;
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

    char str[] = "已上线";
    printf("请输入用户名：");
    fgets(name, sizeof(name), stdin);

    /* User *user = (User*)malloc(sizeof(User));
    memcpy(user->name,name,strlen(name)); */

    Msg *msg = (Msg *)malloc(size);
    msg->type = ENTER;
    memcpy(msg->content, str, strlen(str));
    memcpy(msg->userName, name, strlen(name) - 1);

    char *buffer = (char *)malloc(size);
    memcpy(buffer, msg, size);
    int pos = 0;
    int len;
    while (pos < size)
    {
        len = send(fd, buffer + pos, BUFFSIZE, 0);
        if (len < 0)
        {
            printf("send error\n");
            break;
        }
        pos += len;
    }
    memset(msg, 0, size);
    memset(buffer, 0, size);
    /* free(buffer);
    free(msg); */

    /* send(fd, name, (strlen(name) - 1), 0);
    send(fd, str, (strlen(str)), 0); */

    pthread_t tid;
    pthread_create(&tid, NULL, pthread_recv, NULL);

    while (1)
    {
        memset(sendbuf, 0, sizeof(sendbuf));
        fgets(sendbuf, sizeof(sendbuf), stdin);

        memcpy(msg->userName, name, strlen(name) - 1);

        // 退出
        if (strncmp(sendbuf, "exit", 4) == 0)
        {
            // memset(sendbuf, 0, sizeof(sendbuf));
            printf("您已下线\n");
            char *offline = "已下线";
            memcpy(msg->content, offline, strlen(offline));
            msg->type = EXIT;
        }
        // 升级
        else if (strncmp(sendbuf, "/u", 2) == 0)
        {
            char *split;
            split = strtok(sendbuf, " ");
            // 得到插件名
            split = strtok(NULL, " ");
            // 去除换行符
            split[strlen(split) - 1] = 0;
            runPlugin(split);
            continue;
        }
        // 撤回
        else if (strncmp(sendbuf, "/w", 2) == 0)
        {
            char *split;
            split = strtok(sendbuf, " ");
            // 得到消息内容
            split = strtok(NULL, " ");
            // 去除换行符
            split[strlen(split) - 1] = 0;
            memcpy(msg->content, split, strlen(split));
            // printf("wd content:%s\n",msg->content);
            msg->type = WITHDRAW;
        }
        // 私聊
        else if (strncmp(sendbuf, "/p", 2) == 0)
        {
            char *split;
            split = strtok(sendbuf, " ");
            // 得到消息内容
            split = strtok(NULL, " ");
            memcpy(msg->content, split, strlen(split));
            int i = 0;
            split = strtok(NULL, " ");
            while (split != NULL)
            {
                msg->recvUserId[i++] = atoi(split);
                split = strtok(NULL, " ");
            }
            // printf("id:%d\n", msg->recvUserId[0]);
        }
        // 群聊
        else
        {
            memcpy(msg->content, sendbuf, strlen(sendbuf) - 1);
        }

        // buffer = (char *)malloc(size);
        memcpy(buffer, msg, size);
        int pos = 0;
        int len = 0;
        while (pos < size)
        {
            len = send(fd, buffer + pos, BUFFSIZE, 0);
            if (len <= 0)
            {
                printf("send error\n");
                break;
            }
            pos += len;
        }
        memset(msg, 0, size);
        memset(buffer, 0, size);
        if (strncmp(sendbuf, "exit", 4) == 0)
        {
            exit(0);
        }
        /* free(buffer);
        free(msg); */
        /* char msg[1026];
        send(fd, name, (strlen(name) - 1), 0);
        sprintf(msg, ":%s", sendbuf);
        send(fd, msg, strlen(msg), 0); */
    }
    close(fd);
}

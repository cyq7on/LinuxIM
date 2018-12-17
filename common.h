#ifndef COMMON_H
#define COMMON_H
#define PORT 1234
#define MAXCLIENT 5
#define BUFFSIZE 1024

typedef struct {
    // 发送方fd
    int sendFd;
    // 发送方用户名
    char name[64];
    // 接收方fd
    int recvFd[MAXCLIENT - 1];
    // 消息内容
    char content[1024];
} Msg;
#endif
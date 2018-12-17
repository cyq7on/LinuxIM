#ifndef COMMON_H
#define COMMON_H
#define PORT 1234
#define MAXCLIENT 5
#define BUFFSIZE 1024

typedef struct {
    // 发送方用户名
    char UserName[32];
    // 接收方用户id
    int recvUserId[MAXCLIENT - 1];
    // 消息内容
    char content[1024];
} Msg;

typedef struct {
    long id;
    char name[32];
} User;

#endif
#ifndef COMMON_H
#define COMMON_H

#define PORT 1234
#define MAXCLIENT 7
#define BUFFSIZE 1024
#define HELP "-help"
#define ENTER "-enter"
// 正常的聊天消息类型
#define DEFAULT "-default"
#define EXIT "-exit"

typedef struct {
    // 发送方fd
    int userFd;
    // 发送方用户名
    char userName[32];
    // 接收方用户id
    int recvUserId[MAXCLIENT - 1];
    // 消息内容
    char content[1024];
    // 消息类型
    char type[8];
    // 发送方id,备用
    int userId;
} Msg;

typedef struct {
    long id;
    char name[32];
} User;

int size = sizeof(Msg);

#endif
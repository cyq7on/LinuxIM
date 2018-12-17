#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h>
#include <strings.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <netdb.h>     
#include<string.h>

#define PORT 1234   
#define MAXSIZE 128   

char sendbuf[1024];
char recvbuf[1024];
char name[100];
int fd;

void pthread_recv(void* ptr){
    while(1){
			if ((recv(fd,recvbuf,MAXSIZE,0)) == -1){ 
					printf("recv() error\n"); 
					exit(1); 
			}	 
			printf("%s",recvbuf);
			memset(recvbuf,0,sizeof(recvbuf));
		}
}



int main(int argc, char *argv[]) { 
int  numbytes;   
char buf[MAXSIZE];   
struct hostent *host;       
struct sockaddr_in server;  


if (argc != 2) {        
    argv[1]="127.0.0.1";
    printf("Use default ip address:%s\n",argv[1]); 
} 


if ((host=gethostbyname(argv[1]))==NULL){  
printf("gethostbyname() error\n"); 
exit(1); 
} 

if ((fd=socket(AF_INET, SOCK_STREAM, 0))==-1){ 
printf("socket() error\n"); 
exit(1); 
} 

bzero(&server,sizeof(server));

server.sin_family = AF_INET; 
server.sin_port = htons(PORT); 
server.sin_addr = *((struct in_addr *)host->h_addr);  
if(connect(fd, (struct sockaddr *)&server,sizeof(struct sockaddr))==-1){  
printf("connect() error\n"); 
exit(1); 
} 

printf("connect success\n");
char str[]="已进入聊天室\n";
printf("请输入用户名：");
fgets(name,sizeof(name),stdin);
send(fd,name,(strlen(name)-1),0);
send(fd,str,(strlen(str)),0);


pthread_t tid;
pthread_create(&tid,NULL,pthread_recv,NULL);

while(1)
{
memset(sendbuf,0,sizeof(sendbuf));
fgets(sendbuf,sizeof(sendbuf),stdin);
//if(strstr(sendbuf,"exit")){
if(strncmp(sendbuf,"exit",4) == 0){
    memset(sendbuf,0,sizeof(sendbuf));
    printf("您已退出群聊\n");
    send(fd,sendbuf,(strlen(sendbuf)),0);
    break;
}
send(fd,name,(strlen(name)-1),0);
send(fd,":",1,0);

send(fd,sendbuf,(strlen(sendbuf)),0);


}
//  sprintf("")
//  if ((numbytes=recvfd,buf,MAXSIZE,0)) == -1){ 
// printf("recv() error\n"); 
// exit(1); 
// } 

close(fd);  
}


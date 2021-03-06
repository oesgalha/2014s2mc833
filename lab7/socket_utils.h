#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <errno.h>

#define MAXDATASIZE 40000

int Accept(int listenfd, struct sockaddr_in *clientaddr);
void Bind(int listenfd, struct sockaddr_in servaddr);
void Close(int connection);
void Connect(int sockfd, struct sockaddr_in sockaddress);
struct sockaddr_in Getsockname(int sockfd, struct sockaddr_in sockaddress);
void InetNtop(int family, char* buffer, struct sockaddr_in sockaddress);
void InetPton(int family, char *ipaddress, struct sockaddr_in sockaddress);
void Listen(int listenfd, int listenq);
void Read(int sockfd, char* buffer);
int Socket(int family, int type, int flags);
void Write(int sockfd, char* buffer);
pid_t wait (int *statloc);
pid_t waitpid (pid_t pid, int *statloc, int options);
void sig_chld(int); 
void err_sys(const char* x);
typedef void Sigfunc(int);
Sigfunc * Signal (int signo, Sigfunc *func);


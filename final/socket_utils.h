#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <time.h>
#include <math.h>

#define LISTENQ 10
#define MAXDATASIZE 40000
#define PORT 2015

#define TRUE 1
#define FALSE 0

int Accept(int listenfd, struct sockaddr_in *clientaddr);
void Bind(int listenfd, struct sockaddr_in servaddr);
void Close(int connection);
void Connect(int sockfd, struct sockaddr_in sockaddress);
struct sockaddr_in Getsockname(int sockfd, struct sockaddr_in sockaddress);
void InetNtop(int family, char* buffer, struct sockaddr_in sockaddress);
void InetPton(int family, char *ipaddress, struct sockaddr_in sockaddress);
void Listen(int listenfd, int listenq);
int Read(int sockfd, char* buffer);
int Socket(int family, int type, int flags);
void Write(int sockfd, char* buffer);
void Select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset, struct timeval *timeout);
int Recvfrom(int sockfd, void *msg, size_t n, int flags, struct sockaddr_in *addr);
int Sendto(int sockfd, const void *msg, size_t n, int flags, struct sockaddr_in *addr);
void DgEcho(int sockfd, struct sockaddr_in *addr);
void DgCli(FILE *fp, int sockfd, struct sockaddr_in *addr);

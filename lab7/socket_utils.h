#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#define MAXDATASIZE 40000

int Socket(int family, int type, int flags);
void Connect(int sockfd, struct sockaddr_in sockaddress);
void InetNtop(int family, char* buffer, struct sockaddr_in sockaddress);
void InetPton(int family, char *ipaddress, struct sockaddr_in sockaddress);
struct sockaddr_in Getsockname(int sockfd, struct sockaddr_in sockaddress);
void Read(int sockfd, char* buffer);
void Write(int sockfd, char* buffer);

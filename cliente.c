#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
 
#define MAXDATASIZE 4096
 
// Criar um socket com as opcoes especificadas
// Fecha o programa em caso de erro
int Socket(int family, int type, int flags) {
  int sockfd;
  if ( (sockfd = socket(family, type, flags)) < 0) {
    perror("socket error");
    exit(1);
  } else {
    return sockfd;
  }
}
 
// Tenta conectar um socket local a um outro socket, que pode ser remoto
// Fecha o programa em caso de erro
void Connect(int sockfd, struct sockaddr_in sockaddress) {
  if (connect(sockfd, (struct sockaddr *) &sockaddress, sizeof(sockaddress)) < 0) {
    perror("connect error");
    exit(1);
  }
}
 
// Converte um IP string para a forma binaria da struct sockaddr_in
// Fecha o programa em caso de erro
void InetPton(int family, char *ipaddress, struct sockaddr_in sockaddress) {
  if (inet_pton(family, ipaddress, &sockaddress.sin_addr) <= 0) {
    perror("inet_pton error");
    exit(1);
  }
}
 
// Converte o IP da forma binaria da struct sockaddr_in para uma string
// e armazena em buffer
// Fecha o programa em caso de erro
void InetNtop(int family, char* buffer, struct sockaddr_in sockaddress) {
  if (inet_ntop(family, &sockaddress.sin_addr, buffer, sizeof(char)*MAXDATASIZE) <= 0) {
    perror("inet_ntop error");
    exit(1);
 }
}
 
// Coleta informacoes sobre um socket, retorna o socket com as informacoes preenchidas
// Fecha o programa em caso de erro
struct sockaddr_in Getsockname(int sockfd, struct sockaddr_in sockaddress) {
  socklen_t socksize = sizeof(sockaddress);
  bzero(&sockaddress, sizeof(sockaddress));
  if (getsockname(sockfd, (struct sockaddr *) &sockaddress, &socksize) < 0) {
    perror("getsockname error");
    exit(1);
  }
  return sockaddress;
}
 
// Recebe dados do servidor e escreve em um buffer
// Se retornar algo > 0, ainda ha dados a serem escritos (ultrapassaram o tamanho do buffer)
int Read(int sockfd, char* buffer) {
  return read(sockfd, buffer, MAXDATASIZE);
}
 
/*
   Cliente
   Aplicacao simples de cliente tcp que se conecta num
   IP e PORTA passados por parametro, e escreve na
   saida padrao a resposta do servidor
*/
int main(int argc, char **argv) {
   // Declaracao de variaveis
   // Socket, buffer, buffer de erro
   // e a estrutura sockaddr_in
   int sockfd, n;
   char buf[MAXDATASIZE + 1], error[MAXDATASIZE + 1];
   struct sockaddr_in servaddr, localaddr;
 
   // Checa a presenca do parametro de IP e Porta
   // caso ausente, fecha o programa
   if (argc != 3) {
      strcpy(error,"uso: ");
      strcat(error,argv[0]);
      strcat(error," <IPaddress> <Port>");
      perror(error);
      exit(1);
   }
 
   // AF_INET -> IPv4
   // SOCK_STREAM -> TCP
   sockfd = Socket(AF_INET, SOCK_STREAM, 0);
 
   // Limpa o que estiver no ponteiro do socket que representa o servidor
   // Seta o socket do servidor como IPv4 e seta a porta de conexao para a porta da aplicacao.
   bzero(&servaddr, sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_port   = htons(atoi(argv[2]));
   // Converte o IP recebido na entrada para a forma binária da struct
   InetPton(AF_INET, argv[1], servaddr);
   // Conecta o socket local com o socket servidor
   Connect(sockfd, servaddr);

   // Coletar informacoes sobre o socket local
   localaddr = Getsockname(sockfd, localaddr);
   // Converter IP do socket local para string
   InetNtop(AF_INET, buf, localaddr);    
   // Escrever IP e porta do servidor na saida padrao
   printf("Server: IP %s - Port %d\n", argv[1], atoi(argv[2]));
   // Escrever IP e porta do cliente na saida padrao
   printf("Client: IP %s - Port %d\n", buf, htons(localaddr.sin_port));
    
   // lê uma cadeia de caracteres do teclado
   scanf("%s", buf);
    
   // Imprime a linha de comando digitada pelo usuario
   printf("Linha de comando digitada: %s\n", buf);
    
   // envia os dados lidos ao servidor
   write(sockfd , buf , strlen(buf));
 
   // Enquanto houver dados a serem lidos do servidor,
   // ler e jogar no buffer
   // escrever na saida padrao e fechar o programa em caso de erro
   n = Read(sockfd, buf);
   if (n < 0) {
      perror("read error");
      exit(1);
   }
    
   // Imprime a linha de comando devolvida pelo servidor
   printf("Linha de comando recebida: %s\n", buf);
    
   exit(0);
}
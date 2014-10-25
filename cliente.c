#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define MAXLINE 4096
#define APPLICATIONPORT 1024

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
  if (inet_ntop(family, &sockaddress.sin_addr, buffer, sizeof(char)*MAXLINE) <= 0) {
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

// Escreve dados recebidos de um socket num buffer
// Se retornar algo > 0, ainda ha dados a serem escritos (ultrapassaram o tamanho do buffer)
int Read(int sockfd, char* buffer) {
  return read(sockfd, buffer, MAXLINE);
}

/*
   Cliente
   Aplicacao simples de cliente tcp que se conecta num
   IP passado por parametro, na porta 1024 e escreve na
   saida padrao a resposta do servidor
*/
int main(int argc, char **argv) {
   // Delcaracao de variaveis
   // Socket, buffer, buffer de erro
   // e a estrutura sockaddr_in
   int sockfd, n;
   char buf[MAXLINE + 1], error[MAXLINE + 1];
   struct sockaddr_in servaddr, localaddr;

   // Checa a presenca do parametro de IP
   // caso ausente, fecha o programa
   if (argc != 2) {
      strcpy(error,"uso: ");
      strcat(error,argv[0]);
      strcat(error," <IPaddress>");
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
   servaddr.sin_port   = htons(APPLICATIONPORT);
   // Converte o IP recebido na entrada para a forma binária da struct
   InetPton(AF_INET, argv[1], servaddr);
   // Conecta o socket local com o socket servidor
   Connect(sockfd, servaddr);

   // Enquanto houver dados a serem lidos do servidor,
   // ler e jogar no buffer
   // escrever na saida padrao e fechar o programa em caso de erro
   while ( (n = Read(sockfd, buf)) > 0) {
      buf[n] = 0;
      if (fputs(buf, stdout) == EOF) {
         perror("fputs error");
         exit(1);
      }
   }

   // Coletar informacoes sobre o socket local
   localaddr = Getsockname(sockfd, localaddr);
   // Converter IP do socket local para string
   InetNtop(AF_INET, buf, localaddr);
   // Escrever IP e porta na saida padrao
   printf("Host:\tIP: %s\tport: %d\n", argv[1], APPLICATIONPORT);
   printf("Loocal:\tIP: %s\tport: %d\n", buf, htons(localaddr.sin_port));
   // Se não recebeu nenhum dado, reportar erro de leitura
   // e fechar o programa
   if (n < 0) {
      perror("read error");
      exit(1);
   }

   exit(0);
}

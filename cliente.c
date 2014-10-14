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
   int    sockfd, n;
   char   recvline[MAXLINE + 1];
   char   error[MAXLINE + 1];
   struct sockaddr_in servaddr;

   // Checa a presenca do parametro de IP
   // caso ausente, fecha o programa
   if (argc != 2) {
      strcpy(error,"uso: ");
      strcat(error,argv[0]);
      strcat(error," <IPaddress>");
      perror(error);
      exit(1);
   }

   // Tenta criar um socket local TCP IPv4
   // AF_INET -> IPv4
   // SOCK_STREAM -> TCP
   // Fecha o programa se ocorrer algum erro
   if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      perror("socket error");
      exit(1);
   }

   // Limpa o que estiver no ponteiro do socket que representa o servidor
   // Seta o socket do servidor como IPv4 e seta a porta de conexão para 1024.
   bzero(&servaddr, sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_port   = htons(1024);
   // Tenta converter o IP recebido na entrada (string) para a forma binária da struct
   // Em caso de falha, o programa fecha
   if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) {
      perror("inet_pton error");
      exit(1);
   }

   // Tenta fazer uma conexao com o servidor, usando o socket local
   // E o socket do servidor
   // Em caso de falha, o programa fecha
   if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
      perror("connect error");
      exit(1);
   }

   // Enquanto houver dados a serem lidos do servidor,
   // ler e jogar no buffer
   // escrever na saida padrao e fechar o programa em caso de erro
   while ( (n = read(sockfd, recvline, MAXLINE)) > 0) {
      recvline[n] = 0;
      if (fputs(recvline, stdout) == EOF) {
         perror("fputs error");
         exit(1);
      }
   }

   // Se não recebeu nenhum dado, reportar erro de leitura
   // e fechar o programa
   if (n < 0) {
      perror("read error");
      exit(1);
   }

   exit(0);
}

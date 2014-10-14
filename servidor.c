#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#define LISTENQ 10
#define MAXDATASIZE 100

/*
   Servidor
   Aplicacao simples de servidor tcp que recebe
   conexoes na porta 1024 e responde com o horario
   da maquina rodando a aplicacao do servidor.
*/
int main (int argc, char **argv) {
   // Delcaracao de variaveis
   // Socket, buffer, ticker
   // e a estrutura sockaddr_in
   int    listenfd, connfd;
   struct sockaddr_in servaddr;
   char   buf[MAXDATASIZE];
   time_t ticks;

   // Tenta criar um socket local TCP IPv4
   // AF_INET -> IPv4
   // SOCK_STREAM -> TCP
   // Fecha o programa se ocorrer algum erro
   if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
      perror("socket");
      exit(1);
   }

   // Limpa o que estiver no ponteiro do socket que representa o servidor
   // Seta o socket do servidor como IPv4 e seta a porta de conexao para 1024.
   // Seta uma mascara para aceitar conexoes de qualquer IP
   bzero(&servaddr, sizeof(servaddr));
   servaddr.sin_family      = AF_INET;
   servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
   servaddr.sin_port        = htons(1024);

   // Tentar fazer o bind do socket de servidor na porta escolhida
   // Fechar o programa em caso de erro
   if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
      perror("bind");
      exit(1);
   }
   
   // Setar socket como passivo (aceita conexoes)
   // Em caso de falha, fechar o programa
   if (listen(listenfd, LISTENQ) == -1) {
      perror("listen");
      exit(1);
   }

   // Loop infinito
   for ( ; ; ) {
      // Se chegou uma conexao
      // Em caso de falha fechar o programa
      if ((connfd = accept(listenfd, (struct sockaddr *) NULL, NULL)) == -1 ) {
         perror("accept");
         exit(1);
      }

      // Pegar o relogio da maquina e escrever o horario
      // na resposta enviada ao client conectado
      ticks = time(NULL);
      snprintf(buf, sizeof(buf), "%.24s\r\n", ctime(&ticks));
      write(connfd, buf, strlen(buf));

      // Iniciar finalizacao da conexao
      close(connfd);
   }
   return(0);
}

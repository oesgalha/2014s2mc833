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
 
#define LISTENQ 10
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
 
// Fazer um bind do socket com os parametros escolhidos
// Fechar o programa em caso de erro
void Bind(int listenfd, struct sockaddr_in servaddr) {
    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
      perror("bind");
      exit(1);
   }
}
 
// Setar socket como passivo (aceita conexoes)
// Fechar o programa em caso de erro
void Listen(int listenfd, int listenq) {
    if (listen(listenfd, listenq) == -1) {
      perror("listen");
      exit(1);
   }
}
 
// Aceita a conexao do cliente
// Em caso de falha fechar o programa
int Accept(int listenfd, struct sockaddr_in clientaddr) {
    int connfd, clientsize;
    clientsize = sizeof(clientaddr);
    if ((connfd = accept(listenfd, (struct sockaddr *)&clientaddr, (socklen_t*)&clientsize)) == -1 ) {
        perror("accept");
        exit(1);
    } else {
        return connfd;
    }
}
 
// Recebe dados do cliente e escreve em um buffer
// Se retornar algo > 0, ainda ha dados a serem escritos (ultrapassaram o tamanho do buffer)
void Read(int sockfd, char* buffer) {
    int read_size;
    read_size = recv(sockfd, buffer, MAXDATASIZE, 0);
    if (read_size < 0) {
        perror("read error");
        exit(1);
    }
}
 
// Envia dados do cliente e escreve em um buffer
// Se retornar algo > 0, ainda ha dados a serem escritos (ultrapassaram o tamanho do buffer)
void Write(int sockfd, char* buffer) {
    int write_size;
    write_size = write(sockfd, buffer, strlen(buffer));
    if (write_size < 0) {
        perror("write error");
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

struct sockaddr_in Getpeername(int connfd, struct sockaddr_in sockaddress) {
   socklen_t socksize = sizeof(sockaddress);
   if (getpeername(connfd, (struct sockaddr *) &sockaddress, &socksize) < 0) {
      perror("getsockname error");
      exit(1);
   }
   return sockaddress;
}
 
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
   struct sockaddr_in clientaddr;
   char   buf[MAXDATASIZE + 1], error[MAXDATASIZE + 1];
    
   // Checa a presenca do parametro Porta
   // caso ausente, fecha o programa
   if (argc != 2) {
      strcpy(error,"uso: ");
      strcat(error,argv[0]);
      strcat(error," <Port>");
      perror(error);
      exit(1);
   }
 
   // Tenta criar um socket local TCP IPv4
   // AF_INET -> IPv4
   // SOCK_STREAM -> TCP
   listenfd = Socket(AF_INET, SOCK_STREAM, 0);
 
   // Limpa o que estiver no ponteiro do socket que representa o servidor
   // Seta o socket do servidor como IPv4 e seta a porta de conexao passada por parametro.
   // Seta uma mascara para aceitar conexoes de qualquer IP
   bzero(&servaddr, sizeof(servaddr));
   servaddr.sin_family      = AF_INET;
   servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
   servaddr.sin_port        = htons(atoi(argv[1]));
 
   // Tentar fazer o bind do socket de servidor na porta escolhida
   Bind(listenfd, servaddr);
    
   // Setar socket como passivo (aceita conexoes)
   // Em caso de falha, fechar o programa
   Listen(listenfd, LISTENQ);
 
   // Loop infinito
   for ( ; ; ) {
      // Se chegou uma conexao
      // Em caso de falha fechar o programa
      connfd = Accept(listenfd, clientaddr);
      // Coletar informacoes do socket do client
      clientaddr = Getpeername(connfd, clientaddr);
      // Converter informacao do IP de binario para string
      // armazenar o resultado no buffer
      InetNtop(AF_INET, buf, clientaddr);
      // Escrever IP e porta do client na saida padrao
      printf("Client: IP: %s\tport: %d\n", buf, htons(clientaddr.sin_port));
      bzero(&buf, sizeof(buf));
      Read(connfd, buf);
 
        // Imprime a linha de comando recebida do usuario
        printf("Linha de comando recebida cliente: %s\n", buf);
        // Envia a mensagem de volta para o cliente
        write(connfd, buf, strlen(buf));
        // Executa o comando
        printf("Execucao do Comando:\n");
        system(buf);
        printf("\n");
      // Finalizar a conexao
      close(connfd);
   }
   return(0);
}

#include "socket_utils.h"

int main(int argc, char **argv) {
   // Declaracao de variaveis
   int sockfd;
   char buf[MAXLINE];
   struct sockaddr_in servaddr;

   // Checa a presenca do parametro de IP
   // caso ausente, fecha o programa
   if (argc != 2) {
      strcpy(buf, "uso: ");
      strcat(buf, argv[0]);
      strcat(buf, " <IPaddress>");
      perror(buf);
      exit(1);
   }

   // Limpa o que estiver no ponteiro do socket que representa o servidor
   // Seta o socket do servidor como IPv4 e seta a porta de conexao para a porta da aplicacao.
   bzero(&servaddr, sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_port   = htons(PORT);

   // Converte o IP recebido na entrada para a forma binária da struct
   InetPton(AF_INET, argv[1], servaddr);

   // Cria um socket
   sockfd = Socket(AF_INET, SOCK_DGRAM, 0);

	// Chama a funcao DgCli para fazer o funcionamento de cliente
	dgCli(stdin, sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
   return 0;
}

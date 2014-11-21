/* Cliente TCP */
#include "socket_utils.h"

int main(int argc, char **argv) {
   // Declaracao de variaveis
   int sockfd;//, maxfdp1;
   char buf[MAXDATASIZE + 1];
   struct sockaddr_in servaddr;
   fd_set rset;

   // Checa a presenca do parametro de IP e Porta
   // caso ausente, fecha o programa
   if (argc != 3) {
      strcpy(buf, "uso: ");
      strcat(buf, argv[0]);
      strcat(buf, " <IPaddress> <Port>");
      perror(buf);
      exit(1);
   }

   // Cria um socket
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

   for ( ; ; ) {
      // Resetar rset
      FD_ZERO(&rset);
      FD_SET(STDIN_FILENO, &rset);
      FD_SET(sockfd, &rset);
      // Como o STDIN_FILENO = 0, podemos usar sempre sockfd como valor MAX
      Select(sockfd + 1, &rset, NULL, NULL, NULL);
      // se tem atividade no socket
      if (FD_ISSET(sockfd, &rset)) {
         // le os dados enviados pelo servidor
         Read(sockfd, buf);
         // Imprime o texto devolvida pelo servidor
         printf("%s", buf);
      }
      // se atividade na entrada padrao
      if (FD_ISSET(STDIN_FILENO, &rset)) {
         // le uma cadeia de caracteres da entrada padrao
         if (fgets(buf, MAXDATASIZE, stdin) == NULL) {
            strcpy(buf,"exit\n");
            Write(sockfd , buf);
            break;
         }
         // envia os dados lidos ao servidor
         Write(sockfd, buf);
         sleep(1);
      }
   }
   return 0;
}

/* Cliente TCP */
#include "socket_utils.h"

void ClearStr(char* buffer) {
   int i;
   for(i = 0; i < MAXDATASIZE; i++) {
      buffer[i] = '\0';
   }
}

int main(int argc, char **argv) {
   // Declaracao de variaveis
   int sockfd, reading_input = TRUE, reading_socket = TRUE;
   char buf[MAXDATASIZE + 1], server[MAXDATASIZE];
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

   while(reading_input || reading_socket) {
      // Resetar rset
      FD_ZERO(&rset);
      if (reading_input)
         FD_SET(STDIN_FILENO, &rset);
      if (reading_socket)
         FD_SET(sockfd, &rset);
      // Como o STDIN_FILENO = 0, podemos usar sempre sockfd como valor MAX
      Select(sockfd + 1, &rset, NULL, NULL, NULL);
      // se tem atividade no socket
      if (FD_ISSET(sockfd, &rset)) {
         ClearStr(server);
         // le os dados enviados pelo servidor
         if (Read(sockfd, server) == 0) {
            reading_socket = FALSE;
         } else {
            // Imprime o texto devolvida pelo servidor
            printf("%s", server);
         }
      }
      // se atividade na entrada padrao
      if (FD_ISSET(STDIN_FILENO, &rset)) {
         ClearStr(buf);
         // le uma cadeia de caracteres da entrada padrao
         if (fgets(buf, MAXDATASIZE, stdin) == NULL) {
            shutdown(sockfd, SHUT_WR);
            reading_input = FALSE;
         } else {
            // envia os dados lidos ao servidor
            Write(sockfd, buf);
         }
      }
   }
   return 0;
}

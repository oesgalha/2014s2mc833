#include "socket_utils.h"

void ClearStr(char* buffer) {
   int i;
   for(i = 0; i < MAXLINE; i++) {
      buffer[i] = '\0';
   }
}


// Mostra as instrucoes de uso do programa
void help() {
   char msg[MAXLINE];
   strcpy(msg, "================================================\n");
   strcat(msg, "\nUSAGE:\n");
   strcat(msg, "------------------------------------------------\n");
   strcat(msg, "Connects to the server with an inputed nickname:\n");
   strcat(msg, "/connect <NICKNAME>\n");
   strcat(msg, "------------------------------------------------\n");
   strcat(msg, "List the users connected in the chat:\n");
   strcat(msg, "/list\n");
   strcat(msg, "------------------------------------------------\n");
   strcat(msg, "Start to chat with another user:\n");
   strcat(msg, "/chat <NICKNAME>\n");
   strcat(msg, "================================================\n");
   fputs(msg, stdout);
}

// Trata o text devolvido pelo servidor
void treatServerMsg(int sockfd, struct sockaddr_in servaddr, char *msg) {
   if (strncmp(msg, "/ack", 4) == 0) {
      Sendto(sockfd, "/ack", 4, 0, (struct sockaddr *) &servaddr,  sizeof(servaddr));
   } else {
      fputs(msg, stdout);
   }
}

int main(int argc, char **argv) {
   // Declaracao de variaveis
   int n, sockfd, reading_input = TRUE, reading_socket = TRUE;
   char buf[MAXLINE], server[MAXLINE+1];
   struct sockaddr_in servaddr;
   struct timeval timeout;
   fd_set rset;

   // Checa a presenca do parametro de IP
   // caso ausente, fecha o programa
   if (argc != 2) {
      strcpy(buf, "uso: ");
      strcat(buf, argv[0]);
      strcat(buf, " <IPaddress or Servername>");
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

   // Mostrar instrucoes de uso:
   help();

   while(reading_input || reading_socket) {
      // Resetar rset
      FD_ZERO(&rset);
      // Timeout (2 min sem acao do usuario)
      timeout.tv_sec = 5;
      timeout.tv_usec = 0;
      if (reading_input)
         FD_SET(STDIN_FILENO, &rset);
      if (reading_socket)
         FD_SET(sockfd, &rset);
      // Como o STDIN_FILENO = 0, podemos usar sempre sockfd como valor MAX
      Select(sockfd + 1, &rset, NULL, NULL, &timeout);
      // se tem atividade no socket
      if (FD_ISSET(sockfd, &rset)) {
         ClearStr(server);
         // le os dados enviados pelo servidor
         n = Recvfrom(sockfd, server, MAXLINE, 0, NULL, NULL);
         server[n] = 0;
         // Trata o texto devolvida pelo servidor
         treatServerMsg(sockfd, servaddr, server);
      }
      // se atividade na entrada padrao
      if (FD_ISSET(STDIN_FILENO, &rset)) {
         ClearStr(buf);
         // le uma cadeia de caracteres da entrada padrao
         if (fgets(buf, MAXLINE, stdin) == NULL) {
            reading_input = FALSE;
         } else {
            // envia os dados lidos ao servidor
            Sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *) &servaddr,  sizeof(servaddr));
         }
      }
   }

   // Chama a funcao DgCli para fazer o funcionamento de cliente
   //dgCli(stdin, sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
   return 0;
}

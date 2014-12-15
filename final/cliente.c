#include "socket_utils.h"

int chattingIp;

// limpa o buffer
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
   strcat(msg, "Connects to the chat room with a nickname:\n");
   strcat(msg, "/connect <NICKNAME>\n");
   strcat(msg, "------------------------------------------------\n");
   strcat(msg, "List the users connected in the chat:\n");
   strcat(msg, "/list\n");
   strcat(msg, "------------------------------------------------\n");
   strcat(msg, "Start to chat with another user:\n");
   strcat(msg, "/chat <NICKNAME>\n");
   strcat(msg, "------------------------------------------------\n");
   strcat(msg, "Send a file to the other user:\n");
   strcat(msg, "/file <FILENAME>\n");
   strcat(msg, "------------------------------------------------\n");
   strcat(msg, "Disconnects from the chat room:\n");
   strcat(msg, "/quit\n");
   strcat(msg, "================================================\n");
   fputs(msg, stdout);
}

// funcao que recebe o arquivo
int receiveFile(const char *filename, char *ip) {
   FILE *fp = fopen(filename, "wb");
   if (NULL == fp) {
      printf("Error trying to save the file");
   } else {
      int sockfd = Socket(AF_INET, SOCK_STREAM, 0);

      struct sockaddr_in servaddr;
      servaddr.sin_family = AF_INET;
      servaddr.sin_port = htons(FILEPORT); // port
      // Converte o IP para a forma binaria da struct
      InetPton(AF_INET, ip, servaddr);

      // Conectar
      Connect(sockfd, servaddr);

      // Receber o arquivo e salvar
      int received = 0;
      char buffer[MAXLINE];
      bzero(&buffer, sizeof(buffer));
      while((received = Read(sockfd, buffer)) > 0 ) {
         fwrite(buffer, 1, received, fp);
      }
      printf("File received!\n");
      fclose(fp);
   }
   return 0;
}

// funcao que envia o arquivo
int sendFile(const char *filename) {
   int listenfd = Socket(AF_INET, SOCK_STREAM, 0);

   struct sockaddr_in servaddr, clientaddr;
   bzero(&servaddr, sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_port = htons(FILEPORT);
   servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

   Bind(listenfd, servaddr);

   Listen(listenfd, 10);

   int connfd = Accept(listenfd, &clientaddr);

   /* Open the file that we wish to transfer */
   printf("Sending %s\n", filename);
   FILE *fp = fopen(filename, "rb");
   if (fp == NULL) {
      printf("File not found!");
   } else {
      while(TRUE) {
         int sendAmount;
         char buffer[MAXLINE] = { 0 };
         if ((sendAmount = fread(buffer, 1, MAXLINE, fp)) > 0) {
            Write(connfd, buffer);
         } else {
            printf("File sent!\n");
            break;
         }
      }
      close(connfd);
      fclose(fp);
   }
   return 0;
}

// Trata a mensagem devolvido pelo servidor
void treatServerOutput(int sockfd, struct sockaddr_in servaddr, char *msg) {
   if (strncmp(msg, "/ack", 4) == 0) {
      sendto(sockfd, "/ack", 4, 0, (struct sockaddr *) &servaddr,  sizeof(servaddr));
   } else if(strncmp(msg, "/filereceive", 12) == 0) {
      char userIp[MAXLINE], fileName[MAXLINE];
      strncpy(userIp, &msg[13], MAXLINE-14);
      size_t ln = strlen(userIp) - 1;
      if (userIp[ln] == '\n') userIp[ln] = '\0';
      fputs("You are receiving a file from the other user, please enter a name to save it:\n", stdout);
      fgets(fileName, MAXLINE, stdin);
      ln = strlen(fileName) - 1;
      if (fileName[ln] == '\n') fileName[ln] = '\0';
      receiveFile(fileName, userIp);
   } else if(strncmp(msg, "/file", 5) == 0) {
      char fileName[MAXLINE];
      strncpy(fileName, &msg[6], MAXLINE-7);
      size_t ln = strlen(fileName) - 1;
      if (fileName[ln] == '\n') fileName[ln] = '\0';
      sendFile(fileName);
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
   struct addrinfo hints, *res;
   struct in_addr addr;
   fd_set rset;

   // Checa a presenca do parametro de IP
   // caso ausente, fecha o programa
   if (argc != 2) {
      strcpy(buf, "uso: ");
      strcat(buf, argv[0]);
      strcat(buf, " <IPaddress or Servername>");
      perror(buf);
      exit(0);
   }

   // Limpa o que estiver no ponteiro do socket que representa o servidor
   // Seta o socket do servidor como IPv4 e seta a porta de conexao para a porta da aplicacao.
   bzero(&servaddr, sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_port   = htons(PORT);

   bzero(&hints, sizeof(hints));
   hints.ai_socktype = SOCK_STREAM;
   hints.ai_family = AF_INET;

   if (getaddrinfo(argv[1], NULL, &hints, &res) != 0) {
      printf("Error to resolve the servername\n");
      return 1;
   }

   addr.s_addr = ((struct sockaddr_in *)(res->ai_addr))->sin_addr.s_addr;

   // Converte o IP recebido na entrada para a forma binária da struct
   // InetPton(AF_INET, argv[1], servaddr);

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
         n = recvfrom(sockfd, server, MAXLINE, 0, NULL, NULL);
         server[n] = 0;
         // Trata o texto devolvida pelo servidor
         treatServerOutput(sockfd, servaddr, server);
      }
      // se atividade na entrada padrao
      if (FD_ISSET(STDIN_FILENO, &rset)) {
         ClearStr(buf);
         // le uma cadeia de caracteres da entrada padrao
         if (fgets(buf, MAXLINE, stdin) == NULL) {
            reading_input = FALSE;
         } else {
            // envia os dados lidos ao servidor
            sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *) &servaddr,  sizeof(servaddr));
         }
      }
   }
   return 0;
}

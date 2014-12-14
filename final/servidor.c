#include "socket_utils.h"

// estrutura de no de uma lista que contem um cliente conectado
typedef struct NoClient {
   char *ip;                     // ip do cliente origem
   int port;                     // porta do cliente origem
   char *username;               // username do cliente destino
   char *clidest;                // ip do cliente destino
   int sockfd;                   // sockfd do client origem
   struct sockaddr_in cliaddr;   // dados do socket do cliente origem
   struct NoClient *next;        // ponteiro para o proximo no da lista
} NoClient;

// estrutura de lista que contem dados sobre os clientes conectados
typedef struct Clients {
   int n;                        // numero de clientes
   NoClient *connected;          // vetor de clientes conectados
} Clients;

void clearMem(Clients *cli) {
   NoClient *aux, *client = cli->connected;
   while (client != NULL) {
      aux = client;
      free(aux->ip);
      free(aux->clidest);
      free(aux->username);
      free(aux);
      client = client->next;
   }
   free(cli);
}

// Limpa uma string
void clearStr(char *buffer) {
   int i;
   for(i = 0; i < MAXLINE; i++) {
      buffer[i] = '\0';
   }
}
// inicializa a lista de clientes vazia
Clients *initClients() {
   Clients *cli = (Clients*)malloc(sizeof(Clients));
   cli->n = 0;
   cli->connected = NULL;
   return cli;
}

// conecta cliente com o bate-papo
void clientConnect(Clients *cli, int sockfd, struct sockaddr_in cliaddr, char *ip, int port, char *username) {
   NoClient *newClient = (NoClient*)malloc(sizeof(NoClient));
   newClient->next = cli->connected;
   newClient->ip = ip;
   newClient->port = port;
   newClient->username = username;
   newClient->cliaddr = cliaddr;
   newClient->sockfd = sockfd;
   newClient->clidest = NULL;
   cli->connected = newClient;
   cli->n++;
}

// desconecta cliente com o bate-papo
void clientDesconnect(Clients *cli, char *ip, int port) {
   NoClient *client = cli->connected;
   NoClient *ant;

   // se for o primeiro da lista
   if( (strcmp(client->ip, ip) == 0) && client->port == port) {
      cli->connected = cli->connected->next;
      cli->n--;
      free(client);
   } else {
      ant = client;
      client = client->next;
      while (client != NULL) {
         if( (strcmp(client->ip, ip) == 0) && client->port == port) {
            ant->next = client->next;
            free(client);
            client = ant->next;
            break;
         }
         ant = client;
         client = client->next;
      }
   }
}

// Envia mensagem para um cliente
void sendMsgToClient(NoClient *client, char *msg) {
   socklen_t clilen = sizeof(client->cliaddr);
   printf("Sending msg to client - IP: %s - Port: %d - Msg: %s", client->ip, client->port, msg);
   Sendto(client->sockfd, msg, MAXLINE, 0, (const struct sockaddr *) &(client->cliaddr), clilen);
}

// envia uma string com a lista de clientes conectados no bate-papo
void listClientConnected(Clients *cli) {
   NoClient *client = cli->connected;
   socklen_t clilen = sizeof(client->cliaddr);
   char buf[100], msg[MAXLINE];
   int cont = 1;

   strcpy(msg, "\nUSERS LIST\n");
   while (client != NULL) {
      sprintf(buf, "%d) ", cont);
      strcat(msg, buf);
      strcat(msg, client->username);
      client = client->next;
      cont++;
   }
   Sendto(cli->connected->sockfd, msg, MAXLINE, 0, (const struct sockaddr *) &(cli->connected->cliaddr), clilen);
}

// trata a primeira vez que o cliente se conecta com o bate-papo
void firstConnect(Clients *cli, int sockfd, struct sockaddr_in cliaddr, char *ip, int port, char *msg) {
   char *username = (char*)malloc(MAXLINE * sizeof(char));
   // Descobre o username do cliente
   strncpy(username, &msg[8], MAXLINE-7);
   // conecta o cliente no bate-papo
   clientConnect(cli, sockfd, cliaddr, ip, port, username);
   // envia uma mensagem de sucesso para o cliente e solicita o username
   sendMsgToClient(cli->connected, "Connected!\n");
   //Sendto(sockfd, "Connected!\n", 40, 0, (struct sockaddr *) &cliaddr, clilen);
}

char *getUsernameOrig(Clients *cli, char *ip, int port) {
   NoClient *client = cli->connected;
   while (client != NULL) {
      if( (strcmp(client->ip, ip) == 0) && client->port == port) {
         return client->username;
      }
      client = client->next;
   }
   return NULL;
}

char *getUsernameDest(Clients *cli, char *usernameOrig) {
   NoClient *client = cli->connected;
   while (client != NULL) {
      if(strcmp(client->username, usernameOrig) == 0) {
         return client->clidest;
      }
      client = client->next;
   }
   return NULL;
}

// faz a busca e retorna um no de cliente
NoClient *getClient(Clients *cli, char *username, char *ip, int port) {
   NoClient *client = cli->connected;
   while (client != NULL) {
      if(username) {
         if(strcmp(client->username, username) == 0) {
            break;
         }
      } else {
         if( (strcmp(client->ip, ip) == 0) && client->port == port) {
            break;
         }
      }
      client = client->next;
   }
   return client;
}

// conecta cliente com o outro
void clientConnectclient(Clients *cli, char *ip, int port, char *msg) {
   NoClient *clientOrig, *clientDest;
   socklen_t clilen = sizeof(cli->connected->cliaddr);
   char returnMsg[MAXLINE], usernameDest[MAXLINE];
   memcpy(&usernameDest, &msg[5], MAXLINE);
   int connected = FALSE;

   // busca o cliente de origem
   clientOrig = getClient(cli, NULL, ip, port);

   // busca o cliente de destino
   clientDest = getClient(cli, usernameDest, NULL, 0);
   if(clientDest) {
      clientDest->clidest = clientOrig->username;
      clientOrig->clidest = clientDest->username;
      connected = TRUE;
   } else {
      Sendto(clientOrig->sockfd, "Client Busy\n", 15, 0, (struct sockaddr *) &(clientOrig->cliaddr), clilen);
   }

   // envia uma mensagem de sucesso ou erro para o usuario
   if(connected) {
      sprintf(returnMsg, "Connected with %s", usernameDest);
      Sendto(clientOrig->sockfd, returnMsg, 50, 0, (struct sockaddr *) &clientOrig->cliaddr, clilen);
   } else {
      Sendto(clientOrig->sockfd, "User Not Found\n", 15, 0, (struct sockaddr *) &clientOrig->cliaddr, clilen);
   }
}

// envia uma mensagem para outro cliente
void sendText(Clients *cli, char *ip, int port) {
   NoClient *clientOrig, *clientDest;
   socklen_t clilen = sizeof(cli->connected->cliaddr);
   char init[MAXLINE], msg[MAXLINE];
   char *usernameOrig = getUsernameOrig(cli, ip, port);
   char *usernameDest = getUsernameDest(cli, usernameOrig);

   // busca o cliente de origem
   clientOrig = getClient(cli, usernameOrig, NULL, 0);

   // monta um texto indicando que pode enviar a mensagem
   sprintf(init, "%s: ", usernameOrig);
   Sendto(clientOrig->sockfd, init, MAXLINE, 0, (struct sockaddr *) &clientOrig->cliaddr, clilen);

   // recebe a mensagem do cliente de origem
   Recvfrom(clientOrig->sockfd, msg, MAXLINE, 0, (struct sockaddr *) &clientOrig->cliaddr, &clilen);

   // busca o cliente de destino da mensagem
   clientDest = getClient(cli, usernameDest, NULL, 0);

   // envia a mensagem para o cliente de destino
   Sendto(clientDest->sockfd, init, MAXLINE, 0, (struct sockaddr *) &clientDest->cliaddr, clilen);
   Sendto(clientDest->sockfd, msg, MAXLINE, 0, (struct sockaddr *) &clientDest->cliaddr, clilen);
}

void routerMsg(char *msg, Clients *cli, int sockfd, struct sockaddr_in cliaddr, char *ip, int port) {
   socklen_t clilen = sizeof(cliaddr);

   if(strncmp(msg, "connect", 7) == 0) {
      firstConnect(cli, sockfd, cliaddr, ip, port, msg);
   } else if(strncmp(msg, "list", 4) == 0) {
      listClientConnected(cli);
   } else if(strncmp(msg, "user", 4) == 0) {
      clientConnectclient(cli, ip, port, msg);
      sendText(cli, ip, port);
   } else {
      Sendto(sockfd, "Command not found\n", 20, 0, (struct sockaddr *) &cliaddr, clilen);
   }
}

int main (int argc, char **argv) {
   // Declaracao de variaveis
   int sockfd, port;
   struct sockaddr_in servaddr;
   struct sockaddr_in cliaddr;
   char ip[MAXLINE], msg[MAXLINE];
   socklen_t clilen = sizeof(cliaddr);
   Clients *cli;

   // Tenta criar um socket local UDP IPv4
   sockfd = Socket(AF_INET, SOCK_DGRAM, 0);

   // Limpa o que estiver no ponteiro do socket que representa o servidor
   // Seta o socket do servidor como IPv4 e seta a porta de conexao passada por parametro.
   // Seta uma mascara para aceitar conexoes de qualquer IP
   bzero(&servaddr, sizeof(servaddr));
   servaddr.sin_family      = AF_INET;
   servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
   servaddr.sin_port        = htons(PORT);

   // Tentar fazer o bind do socket de servidor na porta escolhida
   Bind(sockfd, servaddr);

   // Inicializa a lista de clientes
   cli = initClients();

   // Loop infinito
   for ( ; ; ) {
      // Recebe a mensagem enviada por um cliente
      Recvfrom(sockfd, msg, MAXLINE, 0, (struct sockaddr *) &cliaddr, &clilen);

      // transforma o IP para string e a porta para inteiro
      InetNtop(AF_INET, ip, cliaddr);
      port = htons(cliaddr.sin_port);

      // Escrever IP, porta e string do cliente na saida padrao
      printf("Incoming msg from client - IP: %s - Port: %d - Msg: %s", ip, port, msg);

      // faz o tratamento da mensagem
      routerMsg(msg, cli, sockfd, cliaddr, ip, port);

      // Limpa o que estiver no socket do client e as strings
      bzero(&cliaddr, sizeof(cliaddr));
      clearStr(msg);
   }
   // libera a memoria
   clearMem(cli);
   return 0;
}

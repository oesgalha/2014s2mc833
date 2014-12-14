#include "socket_utils.h"

// estrutura de no de uma lista que contem um cliente conectado
typedef struct NoClient {
   char *ip;                     // ip do cliente origem
   int port;                     // porta do cliente origem
   char *username;               // username do cliente destino
   struct NoClient *clidest;     // cliente destino
   int sockfd;                   // sockfd do client origem
   int anonymous;                // boolean para identificar se e um usuario conectado ou nao
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

NoClient *buildClient(int sockfd, struct sockaddr_in cliaddr, char *ip, int port) {
   NoClient *newClient = (NoClient*)malloc(sizeof(NoClient));
   newClient->ip = ip;
   newClient->port = port;
   newClient->cliaddr = cliaddr;
   newClient->sockfd = sockfd;
   newClient->anonymous = TRUE;
   return newClient;
}

// conecta cliente com o bate-papo
void clientConnect(Clients *cli, NoClient *newClient, char *username) {
   newClient->next = cli->connected;
   newClient->username = username;
   newClient->anonymous = FALSE;
   newClient->clidest = NULL;
   cli->connected = newClient;
   cli->n++;
}

// desconecta cliente com o bate-papo
void clientDisconnect(Clients *cli, NoClient *client) {
   NoClient *auxClient = cli->connected;
   NoClient *ant;

   if (client->clidest != NULL) {
      client->clidest->clidest = NULL;
      client->clidest = NULL;
   }

   // se for o primeiro da lista
   if( (strcmp(auxClient->ip, client->ip) == 0) && auxClient->port == client->port) {
      cli->connected = cli->connected->next;
      cli->n--;
      free(auxClient);
   } else {
      ant = auxClient;
      auxClient = auxClient->next;
      while (client != NULL) {
         if( (strcmp(auxClient->ip, client->ip) == 0) && auxClient->port == client->port) {
            ant->next = auxClient->next;
            free(auxClient);
            auxClient = ant->next;
            break;
         }
         ant = auxClient;
         auxClient = auxClient->next;
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
void listClientConnected(Clients *cli, NoClient *client) {
   NoClient *auxClient = cli->connected;
   char buf[100], msg[MAXLINE];
   int cont = 1;

   strcpy(msg, "\nUSERS LIST\n");
   while (auxClient != NULL) {
      sprintf(buf, "%d) ", cont);
      strcat(msg, buf);
      strcat(msg, auxClient->username);
      auxClient = auxClient->next;
      cont++;
   }
   sendMsgToClient(client, msg);
}

// trata a primeira vez que o cliente se conecta com o bate-papo
void firstConnect(Clients *cli, NoClient *client, char *msg) {
   char *username = (char*)malloc(MAXLINE * sizeof(char));
   // Descobre o username do cliente
   strncpy(username, &msg[9], MAXLINE-8);
   // conecta o cliente no bate-papo
   clientConnect(cli, client, username);
   // envia uma mensagem de sucesso para o cliente e solicita o username
   sendMsgToClient(cli->connected, "Connected!\n");
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
void clientConnectclient(Clients *cli, NoClient *clientOrig, char *msg) {
   NoClient *clientDest;
   char returnMsg[MAXLINE], usernameDest[MAXLINE];
   strncpy(usernameDest, &msg[6], MAXLINE-7);

   // busca o cliente de destino
   clientDest = getClient(cli, usernameDest, NULL, 0);
   if (clientDest) {
      if (clientDest->clidest != NULL) {
         sendMsgToClient(clientOrig, "This user is busy chatting with someone else.\n");
      } else {
         clientDest->clidest = clientOrig;
         clientOrig->clidest = clientDest;
         sprintf(returnMsg, "Chatting with %s", usernameDest);
         sendMsgToClient(clientOrig, returnMsg);
         sprintf(returnMsg, "Chatting with %s", clientOrig->username);
         sendMsgToClient(clientDest, returnMsg);
      }
   } else {
      sendMsgToClient(clientOrig, "User Not Found\n");
   }
}


void chatMessage(NoClient *clientOrig, char* msg) {
   char sendMsg[MAXLINE];
   strcpy(sendMsg, clientOrig->username);
   size_t ln = strlen(sendMsg) - 1;
   if (sendMsg[ln] == '\n') sendMsg[ln] = '\0';
   strcat(sendMsg, ": ");
   strcat(sendMsg, msg);
   sendMsgToClient(clientOrig->clidest, sendMsg);
}

void routerMsg(char *msg, Clients *cli, int sockfd, struct sockaddr_in cliaddr, char *ip, int port) {
   NoClient *clientOrig = getClient(cli, NULL, ip, port);
   if (clientOrig == NULL) {
      clientOrig = buildClient(sockfd, cliaddr, ip, port);
   }

   if(strncmp(msg, "/connect", 8) == 0) {
      firstConnect(cli, clientOrig, msg);
   } else if(strncmp(msg, "/list", 5) == 0) {
      listClientConnected(cli, clientOrig);
   } else if(strncmp(msg, "/chat", 4) == 0) {
      if (clientOrig->anonymous) {
         sendMsgToClient(clientOrig, "You must connect first to be able to chat\n");
      } else {
         clientConnectclient(cli, clientOrig, msg);
      }
   } else if(strncmp(msg, "/quit", 4) == 0) {
      if (clientOrig->anonymous) {
         sendMsgToClient(clientOrig, "You must connect first to be able to disconnect\n");
      } else {
         clientDisconnect(cli, clientOrig);
         sendMsgToClient(clientOrig, "Disconnected\n");
      }
   } else if(msg[0] !=  '/' && clientOrig->clidest != NULL) {
      chatMessage(clientOrig, msg);
   } else {
      sendMsgToClient(clientOrig, "Command not found\n");
   }

   if (clientOrig->anonymous) {
      free(clientOrig);
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

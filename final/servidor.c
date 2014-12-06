#include "socket_utils.h"

// estrutura de no de uma lista que contem um cliente conectado
typedef struct NoClient {
	char *ip;					// ip do cliente origem
	int port;					// porta do cliente origem
	char *username;			// username do cliente destino
	char *clidest;				// ip do cliente destino
	struct NoClient *next;	// ponteiro para o proximo no da lista
} NoClient;

// estrutura de lista que contem dados sobre os clientes conectados
typedef struct Clients {
	int n;					// numero de clientes
	NoClient *connected;	// vetor de clientes conectados
} Clients;

// inicializa a lista de clientes vazia
Clients *initClients() {
	Clients *cli = (Clients*)malloc(sizeof(Clients));
	cli->n = 0;
	cli->connected = NULL;
	return cli;
}

// conecta cliente com o bate-papo
void clientConnect(Clients *cli, char *ip, int port, char *username) {
	NoClient *newClient = (NoClient*)malloc(sizeof(NoClient));
	newClient->next = cli->connected;
	newClient->ip = ip;
	newClient->port = port;
	newClient->username = username;
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
	}
	else {
		ant = client;
		client = client->next;
		while (client != NULL)
		{
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

// envia uma string com a lista de clientes conectados no bate-papo
void listClientConnected(Clients *cli, int sockfd, struct sockaddr_in cliaddr) {
	socklen_t clilen = sizeof(cliaddr);
	NoClient *client = cli->connected;
	char buf[100], msg[MAXLINE];
	int cont = 1;
	
	strcpy(msg, "\nUSERS LIST\n");
	while (client != NULL)
	{
		sprintf(buf, "%d) ", cont);
		strcat(msg, buf);
		strcat(msg, client->username);
		client = client->next;
		cont++;
	}
	strcat(msg,"\nEnter username you want to chat: ");
	Sendto(sockfd, msg, MAXLINE, 0, (struct sockaddr *) &cliaddr, clilen);
}

void clearMem(Clients *cli) {
	NoClient *aux, *client = cli->connected;
	while (client != NULL)
	{
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

int main (int argc, char **argv) {
	// Declaracao de variaveis
	int n, sockfd, port;
	struct sockaddr_in servaddr;
	struct sockaddr_in cliaddr;
	char ip[MAXLINE], msg[MAXLINE], *username;
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
		n = Recvfrom(sockfd, msg, MAXLINE, 0, (struct sockaddr *) &cliaddr, &clilen);
		
		// transforma o IP para string e a porta para inteiro
      InetNtop(AF_INET, ip, cliaddr);
      port = htons(cliaddr.sin_port);
      
      // Escrever IP, porta e string do cliente na saida padrao
      printf("Client - IP: %s - Port: %d - Msg: %s", ip, port, msg);
      
      // verifica se eh a primeira vez que o cliente se conecta com o bate-papo
      if(strncmp(msg, "connect", 7) == 0) {
      	// envia uma mensagem de sucesso parao cliente e solicita o username
      	Sendto(sockfd, "Connected!\nInsert username: ", 40, 0, (struct sockaddr *) &cliaddr, clilen);
      	
      	// Recebe o username enviado pelo cliente
      	username = (char*)malloc(MAXLINE * sizeof(char));
			Recvfrom(sockfd, username, MAXLINE, 0, (struct sockaddr *) &cliaddr, &clilen);
      	
      	// conecta o cliente no bate-papo
      	clientConnect(cli, ip, port, username);
      	
      	// Envia a mensagem com a lista de todos os usuarios para o cliente
      	listClientConnected(cli, sockfd, cliaddr);
      }
      
      // Envia a mensagem para o cliente
		Sendto(sockfd, msg, n, 0, (struct sockaddr *) &cliaddr, clilen);	
		
		// Limpa o que estiver no socket do client e as strings
      bzero(&cliaddr, sizeof(cliaddr));
      clearStr(msg);
	}
	
	// libera a memoria
	clearMem(cli);
	
   return 0;
}

/* Servidor TCP */
#include "socket_utils.h"


// Limpa uma string
void ClearStr(char* buffer) {
	int i;
	for(i = 0; i < MAXDATASIZE; i++) {
		buffer[i] = '\0';
	}
}

int main (int argc, char **argv) {
   // Declaracao de variaveis
   int listenfd, connfd;
   pid_t pid;
   struct sockaddr_in servaddr;
   struct sockaddr_in clientaddr;
   char   buf[MAXDATASIZE], error[MAXDATASIZE], client[MAXDATASIZE],
   	openClient[MAXDATASIZE], closeClient[MAXDATASIZE];
   time_t ticks;
   
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
      connfd = Accept(listenfd, &clientaddr);
      
      // Pegar o horario de conexao do cliente
      // limpa o buffer
		ClearStr(openClient);
		ClearStr(closeClient);
      ticks = time(NULL);
      snprintf(openClient, MAXDATASIZE, "%.24s\r", ctime(&ticks));
      
      // cria um processo filho (copia identica do pai)
      if( (pid = fork()) == 0) {
      	// fecha a conexão com o processo pai
      	Close(listenfd);
      	      
		   // Converter informacao do IP de binario para string
		   // armazenar o resultado no buffer
		   InetNtop(AF_INET, buf, clientaddr);   
		   
		   // Escrever IP, porta e string do cliente na saida padrao
	  		printf("OPEN -> Client - IP: %s - Port: %d\n", buf, htons(clientaddr.sin_port));
						   	
			// enquanto o comando for diferente de exit
			do {
				// limpa o buffer
				ClearStr(client);
			
				// Recebe o comando do cliente
				Read(connfd, client);
				
				// Escrever string do cliente na saida padrao
		  		printf("TEXTO -> \n%s", client);
		  		
      		// Envia a linha da mensagem de volta para o cliente
				Write(connfd, client);
								
      	} while(!feof(client)); //while(strcmp(client, "EOF"));
      	
      	// fecha a conexão do processo filho
      	Close(connfd);
      	
      	// Pegar o horario de conexao do cliente
		   ticks = time(NULL);
		   snprintf(closeClient, MAXDATASIZE, "%.24s\r\n", ctime(&ticks));
      	
	   	// Escrever IP, porta e string do cliente que se desconectou
	  		printf("CLOSE-> Client - IP: %s - Port: %d\n", buf, htons(clientaddr.sin_port));
	  		
	  		// Limpa o que estiver no ponteiro do socket do client
			bzero(&clientaddr, sizeof(clientaddr));
   		exit(0);
		}
		
      // Finalizar a conexao
      Close(connfd);
   }
	
   return(0);
}

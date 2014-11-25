/* Cliente TCP */
#include "socket_utils.h"

int max(int v1, int v2) {
	if(v1 >= v2) 
		return v1;
	else
		return v2;
}

int main(int argc, char **argv) {
   // Declaracao de variaveis
   int sockfd, maxfdp1;
   char buf[MAXDATASIZE + 1], error[MAXDATASIZE + 1];
   char server[MAXDATASIZE + 1], server_reply[MAXDATASIZE + 1];
   struct sockaddr_in servaddr;
   fd_set rset;

   // Checa a presenca do parametro de IP e Porta
   // caso ausente, fecha o programa
   if (argc != 3) {
      strcpy(error,"uso: ");
      strcat(error,argv[0]);
      strcat(error," <IPaddress> <Port>");
      perror(error);
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
   
  	// Escrever IP e porta do servidor na saida padrao
   printf("Server - IP: %s - Port: %d\n", argv[1], atoi(argv[2]));
   
   // Coletar informacoes sobre o socket com o servidor
   servaddr = Getsockname(sockfd, servaddr);

   // Converter informacao do IP de binario para string
   // armazenar o resultado no buffer
   InetNtop(AF_INET, server, servaddr);
  	
  	// Escrever IP e porta do cliente no socket na saida padrao
  	printf("Client - IP: %s - Port: %d\n", server, ntohs(servaddr.sin_port));
  	
  /*	do {
  		// limpa o buffer
   	ClearStr(buf);
   	ClearStr(server_reply);
  	
		// lê uma cadeia de caracteres do teclado
		fgets(buf, MAXDATASIZE, stdin);
		
		// envia os dados lidos ao servidor
		Write(sockfd , buf);

		// le os dados enviados pelo servidor
		Read(sockfd, server_reply);
		
		// Imprime a linha de comando devolvida pelo servidor
   	printf("%s\n", server_reply);  	
   	
	} while(!feof(stdin));*/
	
	FD_ZERO(&rset); 
	do { 
		FD_SET(fileno(stdin), &rset); 
		FD_SET(sockfd, &rset); 
		maxfdp1 = max(fileno(stdin), sockfd) + 1; 
		Select(maxfdp1, &rset, NULL, NULL, NULL); 
		// se tem atividade no socket
		if (FD_ISSET(sockfd, &rset)) { 
			// le os dados enviados pelo servidor
			Read(sockfd, server_reply);
			// Imprime o texto devolvida pelo servidor
			printf("%s\n", server_reply); 
		} 
		// se atividade na entrada padrão
		if (FD_ISSET(fileno(stdin), &rset)) { 
			// lê uma cadeia de caracteres da entrada padrão
			fgets(buf, MAXDATASIZE, stdin);
			// envia os dados lidos ao servidor
			Write(sockfd , buf);
		} 
	} while(!feof(stdin)); 
   exit(0);
}

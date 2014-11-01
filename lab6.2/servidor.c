#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#define LISTENQ 10
#define MAXDATASIZE 4096

// Criar um socket com as opcoes especificadas
// Fecha o programa em caso de erro
int Socket(int family, int type, int flags) {
	int sockfd;
	if ( (sockfd = socket(family, type, flags)) < 0) {
	 	perror("socket error");
	 	exit(1);
	} else {
	  return sockfd;
	}
}

// Fazer um bind do socket com os parametros escolhidos
// Fechar o programa em caso de erro
void Bind(int listenfd, struct sockaddr_in servaddr) {
	if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
      perror("bind");
      exit(1);
   	}
}

// Setar socket como passivo (aceita conexoes)
// Fechar o programa em caso de erro
void Listen(int listenfd, int listenq) {
	if (listen(listenfd, listenq) == -1) {
      perror("listen");
      exit(1);
   }
}

// Aceita a conexao do cliente
// Em caso de falha fechar o programa
int Accept(int listenfd, struct sockaddr_in *clientaddr) {
	int connfd, clientsize;
	clientsize = sizeof(clientaddr);
	if ((connfd = accept(listenfd, (struct sockaddr *)clientaddr, (socklen_t*)&clientsize)) == -1 ) {
		perror("accept");
		exit(1);
	} else {
		return connfd;
	}
}

void ClearStr(char* buffer) {
	int i;
	for(i = 0; i < MAXDATASIZE; i++) {
		buffer[i] = '\0';
	}
}

// Recebe dados do cliente e escreve em um buffer
// Se retornar algo > 0, ainda ha dados a serem escritos (ultrapassaram o tamanho do buffer)
void Read(int sockfd, char* buffer) {
	int read_size;
	ClearStr(buffer);
	read_size = recv(sockfd, buffer, MAXDATASIZE, 0);
	if (read_size < 0) {
		perror("read error");
		exit(1);
	}
}

// Envia dados do cliente e escreve em um buffer
// Se retornar algo > 0, ainda ha dados a serem escritos (ultrapassaram o tamanho do buffer)
void Write(int sockfd, char* buffer) {
	int write_size;
	ClearStr(buffer);
	write_size = write(sockfd, buffer, strlen(buffer));
	if (write_size < 0) {
		perror("write error");
		exit(1);
	}
}

// Converte um IP string para a forma binaria da struct sockaddr_in
// Fecha o programa em caso de erro
void InetPton(int family, char *ipaddress, struct sockaddr_in sockaddress) {
  if (inet_pton(family, ipaddress, &sockaddress.sin_addr) <= 0) {
    perror("inet_pton error");
    exit(1);
  }
}

// Converte o IP da forma binaria da struct sockaddr_in para uma string
// e armazena em buffer
// Fecha o programa em caso de erro
void InetNtop(int family, char* buffer, struct sockaddr_in sockaddress) {
  	if (inet_ntop(family, &sockaddress.sin_addr, buffer, sizeof(char)*MAXDATASIZE) <= 0) {
    	perror("inet_ntop error");
    	exit(1);
 	}
}

// fecha a conexão
void Close(int connection) {
	close(connection);
}

// Envia o resultado do comando para o cliente
void WriteCmd(int connfd, char *client) {
	int backup, p[2];
	char buf[MAXDATASIZE], cmd[MAXDATASIZE];
	ClearStr(buf);
	ClearStr(cmd);
	
	printf("writecmd %s", client);
	
	fflush(stdin);
	backup = dup(1);
	Close(0);
	Close(1);
	pipe(p);
	system(client);
	dup2(backup, 1);

	while (fgets(buf, MAXDATASIZE, stdin)){
		strcat(cmd, buf);
	}
	//printf("%s\n", cmd);
	write(connfd, cmd, strlen(cmd));
}

/*
   Servidor
   Aplicacao simples de servidor tcp que recebe varias
   conexoes na porta passada por parametro e executa
   o comando enviado pelo cliente
*/
int main (int argc, char **argv) {
   // Declaracao de variaveis
   int listenfd, connfd;
   pid_t pid;
   struct sockaddr_in servaddr;
   struct sockaddr_in clientaddr;
   char   buf[MAXDATASIZE], error[MAXDATASIZE], client[MAXDATASIZE];
   
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
      
      if( (pid = fork()) == 0) {
      	// fecha a conexão de escuta
      	Close(listenfd);
      	      
      	 printf("OPEN\n");
      	 
		   // Converter informacao do IP de binario para string
		   // armazenar o resultado no buffer
		   InetNtop(AF_INET, buf, clientaddr);   
		   
		   printf("OPEN %s\n", buf);
		   
		   // Escrever IP, porta e string do cliente na saida padrao
	  		printf("OPEN -> Client: IP %s - Port: %d", buf, htons(clientaddr.sin_port));
						   	
			// enquanto o comando for diferente de exit
			do {
				// Recebe o comando do cliente
				Read(connfd, client);
				
				// Escrever IP, porta e string do cliente na saida padrao
		  		printf(" CMD -> Client: IP %s - Port: %d - String: %s", buf, htons(clientaddr.sin_port), client);
		  		
      		// Envia a mensagem de volta para o cliente com o resultado do comando executado
				WriteCmd(connfd, client);
				
      	} while(strcmp(client, "exit\n"));
      	
      	// fecha a conexão do cliente
      	Close(connfd);
	   	// Escrever IP, porta e string do cliente que se desconectou
	  		printf("CLOSE-> Client: IP %s - Port: %d\n", buf, htons(clientaddr.sin_port));
	  		
	  		// Limpa o que estiver no ponteiro do socket do client
			bzero(&clientaddr, sizeof(clientaddr));
   		exit(0);
		}
		
      // Finalizar a conexao
      Close(connfd);
   }
   return(0);
}

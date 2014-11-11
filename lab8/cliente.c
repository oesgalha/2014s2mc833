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

#define MAXDATASIZE 40000

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

// Tenta conectar um socket local a um outro socket, que pode ser remoto
// Fecha o programa em caso de erro
void Connect(int sockfd, struct sockaddr_in sockaddress) {
	if (connect(sockfd, (struct sockaddr *)&sockaddress, sizeof(sockaddress)) < 0) {
		perror("connect error");
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

// Coleta informacoes locais sobre um socket, retorna o socket com as informacoes preenchidas
// Fecha o programa em caso de erro
struct sockaddr_in Getsockname(int sockfd, struct sockaddr_in sockaddress) {
	socklen_t socksize = sizeof(sockaddress);
	bzero(&sockaddress, sizeof(sockaddress));
	if (getsockname(sockfd, (struct sockaddr *) &sockaddress, &socksize) < 0) {
		perror("getsockname error");
		exit(1);
	}
	return sockaddress;
}

// Limpa uma string
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
	write_size = write(sockfd, buffer, strlen(buffer));
	if (write_size < 0) {
		perror("write error");
		exit(1);
	}
}

/*
   Cliente
   Aplicacao simples de cliente tcp que se conecta num
   IP e PORTA passados por parametro, envia um comando ao 
   servidor e escreve na saida padrao o retorno do comando
*/
int main(int argc, char **argv) {
   // Declaracao de variaveis
   int sockfd;
   char buf[MAXDATASIZE + 1], error[MAXDATASIZE + 1];
   char server[MAXDATASIZE + 1], server_reply[MAXDATASIZE + 1];
   struct sockaddr_in servaddr;

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
  	
  	printf("Digite os comandos:\n");
  	do {
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
   	
	} while(strcmp(buf, "exit\n"));
   
   exit(0);
}

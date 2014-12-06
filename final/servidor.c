#include "socket_utils.h"

int main (int argc, char **argv) {
	// Declaracao de variaveis
	int sockfd;
	struct sockaddr_in servaddr;
	struct sockaddr_in cliaddr;

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

	// Chama a funcao DgEcho para fazer o funcionamento do servidor
	dgEcho(sockfd, (struct sockaddr *) &cliaddr, sizeof(cliaddr));	
   return 0;
}

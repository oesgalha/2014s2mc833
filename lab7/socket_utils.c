#include "socket_utils.h"

// Aceita a conexao do cliente
// Em caso de falha fechar o programa
int Accept(int listenfd, struct sockaddr_in *clientaddr) {
  int connfd, clientsize = sizeof(clientaddr);
  if ((connfd = accept(listenfd, (struct sockaddr *)clientaddr, (socklen_t*)&clientsize)) == -1 ) {
    perror("accept");
    exit(1);
  }
  return connfd;
}

// Fazer um bind do socket com os parametros escolhidos
// Fechar o programa em caso de erro
void Bind(int listenfd, struct sockaddr_in servaddr) {
  if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
    perror("bind");
    exit(1);
  }
}

// Fecha a conexao
void Close(int connection) {
  close(connection);
}

// Tenta conectar um socket local a um outro socket, que pode ser remoto
// Fecha o programa em caso de erro
void Connect(int sockfd, struct sockaddr_in sockaddress) {
  if (connect(sockfd, (struct sockaddr *)&sockaddress, sizeof(sockaddress)) < 0) {
    perror("connect error");
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

// Converte o IP da forma binaria da struct sockaddr_in para uma string
// e armazena em buffer
// Fecha o programa em caso de erro
void InetNtop(int family, char* buffer, struct sockaddr_in sockaddress) {
  if (inet_ntop(family, &sockaddress.sin_addr, buffer, sizeof(char)*MAXDATASIZE) <= 0) {
    perror("inet_ntop error");
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

// Setar socket como passivo (aceita conexoes)
// Fechar o programa em caso de erro
void Listen(int listenfd, int listenq) {
  if (listen(listenfd, listenq) == -1) {
    perror("listen");
    exit(1);
  }
}

// Recebe dados do cliente e escreve em um buffer
// Se retornar algo > 0, ainda ha dados a serem escritos (ultrapassaram o tamanho do buffer)
void Read(int sockfd, char* buffer) {
  int read_size = recv(sockfd, buffer, MAXDATASIZE, 0);
  if (read_size < 0) {
    perror("read error");
    exit(1);
  }
}

// Criar um socket com as opcoes especificadas
// Fecha o programa em caso de erro
int Socket(int family, int type, int flags) {
  int sockfd;
  if ( (sockfd = socket(family, type, flags)) < 0) {
    perror("socket error");
    exit(1);
  }
  return sockfd;
}

// Envia dados do cliente e escreve em um buffer
// Se retornar algo > 0, ainda ha dados a serem escritos (ultrapassaram o tamanho do buffer)
void Write(int sockfd, char* buffer) {
  int write_size = write(sockfd, buffer, strlen(buffer));
  if (write_size < 0) {
    perror("write error");
    exit(1);
  }
}

// Função que trata o sig_chld
void sig_chld(int signo) { 
	pid_t pid; 
	int stat; 
	while ( (pid = waitpid(-1, &stat, WNOHANG)) > 0) 
		printf("child %d terminated\n", pid); 
	return; 
}

// função para tratar erro
void err_sys(const char* x) 
{ 
    perror(x); 
    exit(1); 
}

// função para tratar o sinal
Sigfunc * Signal (int signo, Sigfunc *func) 
{ 
	struct sigaction act, oact; 
	act.sa_handler = func; 
	sigemptyset (&act.sa_mask); /* Outros sinais não são bloqueados*/ 
	act.sa_flags = 0; 
	if (signo == SIGALRM) { /* Para reiniciar chamadas interrompidas */
#ifdef SA_INTERRUPT 
		act.sa_flags |= SA_INTERRUPT; /* SunOS 4.x */ 
#endif 
	} else { 
#ifdef SA_RESTART 
		act.sa_flags |= SA_RESTART; /* SVR4, 4.4BSD */ 
#endif 
	} 
	if (sigaction (signo, &act, &oact) < 0) 
		return (SIG_ERR); 
	return (oact.sa_handler); 
}

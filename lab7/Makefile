CC=gcc
CFLAGS= -Wall

all: servidorbin clientebin

clientebin: cliente.c socket_utils.c
	$(CC) $(CFLAGS) -o cliente cliente.c socket_utils.c

servidorbin: servidor.c
	$(CC) $(CFLAGS) -o servidor servidor.c

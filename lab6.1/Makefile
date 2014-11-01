CC=gcc
CFLAGS= -Wall

all: servidorbin clientebin

clientebin: cliente.c
	$(CC) $(CFLAGS) cliente.c -o cliente

servidorbin: servidor.c
	$(CC) $(CFLAGS) servidor.c -o servidor

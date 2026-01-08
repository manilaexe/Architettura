#define _POSIX_C_SOURCE 200112L
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include "utils.h"
#define SIZE 4096

int main(int argc, char *argv[]){

	int sd, err;
	struct addrinfo hints, *res;

	//controllo degli argomenti
	if(argc!=2){
		fprintf(stderr, "Uso: rstrcmp porta");\
		exit(1);
	}

	//direttive per getaddrinfo
	memset(&hints, 0, sizeof(hints));
	hints.ai_family=AF_UNSPEC;
	hints.ai_socktype=SOCK_STREAM;
	hints.ai_flags=AI_PASSIVE;

	//connessione
	err=getaddrinfo(NULL, argv[1], &hints, &res);
	if(err!=0){
		fprintf(stderr, "Errore gai");
		exit(EXIT_FAILURE);
	}

	sd=socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if(err<0){
		fprintf(stderr, "Errore connessione");
		exit(EXIT_FAILURE);
	}

	err=bind(sd, res->ai_addr, res->ai_addrlen);
	if(err<0){
		fprintf(stderr, "Errore bind");
		exit(EXIT_FAILURE);
	}

	freeaddrinfo(res);

	err=listen(sd, SOMAXCONN);
	if(err<0){
		fprintf(stderr, "Errore listen");
		exit(EXIT_FAILURE);
	}

	while(1){
		int ns, pid;
		rxb_buf buf;
		size_t len;
		char s1[SIZE], s2[SIZE];
		
		ns=accept(sd, NULL, NULL);
		if(ns<0){
			continue;
		}

		//gestione della richiesta di servizio
		//inizializzazione del buffer
		rxb_init(&buf, SIZE*2);
		rxb_readline(&buf, ns, s1, SIZE);
		rxb_readline(&buf, ns, s2, SIZE);

		if(strcmp(s1,s2)==0){
			write(ns, "SI\n", 3);
		}else{
			write(ns, "NO\n", 3);
		}

		close(ns);
		rxb_destroy(&buf);
	}
	return 0;
}

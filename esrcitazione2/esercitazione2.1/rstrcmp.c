//Interfaccia: rstrcmp hostname porta   stringa1  stringa2
//             argv[0] argv[1]  argv[2] argv[3]   argv[4]

#define _POSIX_C_SOURCE 200112L
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include "utils.h"
#define SIZE 4096

int main(int argc, char *argv[]){

	//controllo dei parametri
	if(argc!=5){
		fprintf(stderr, "Uso: rstrcmp hostname porta stringa1 stringa2");
		exit(1);
	}

	int sd, err;
	struct addrinfo hints, *res, *ptr;
	char s1[SIZE], s2[SIZE], buffer[SIZE];
	size_t nread;
	strcpy(s1, argv[3]);
	strcpy(s2, argv[4]);

	//direttive
	memset(&hints, 0, sizeof(hints));
	hints.ai_family=AF_UNSPEC;
	hints.ai_socktype=SOCK_STREAM;

	//tentativo di connessione	
	err=getaddrinfo(argv[1], argv[2], &hints, &res);
	if(err!=0){
		fprintf(stderr, "Errore gai");
		exit(EXIT_FAILURE);
	}

	for(ptr=res; ptr!=NULL; ptr=ptr->ai_next){
		sd=socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if(sd<0){
			continue;
		}
		err=connect(sd, ptr->ai_addr, ptr->ai_addrlen);
		if(err==0){
			break;
		}
		close(sd);
	}
	if(ptr==NULL){
		fprintf(stderr, "Errore connessione");
		exit(EXIT_FAILURE);
	}

	freeaddrinfo(res);

	err=write(sd, s1, strlen(s1));
	if(err<0){
		fprintf(stderr, "Errore stringa1");
		exit(EXIT_FAILURE);
	}
	err=write(sd, "\n", 1);
	if(err<0){
		fprintf(stderr, "Errore stringa1");
		exit(EXIT_FAILURE);
	}
	err=write(sd, s2, strlen(s2));
	if(err<0){
		fprintf(stderr, "Errore stringa1");
		exit(EXIT_FAILURE);
	}		
	err=write(sd, "\n", 1);
	if(err<0){
		fprintf(stderr, "Errore stringa1");
		exit(EXIT_FAILURE);
	}
	//lettura della risposta
	while((nread=read(sd, buffer, sizeof(buffer)))>0){
		err=write(1, buffer, nread);
		if(err<0){
			fprintf(stderr, "Errore stringa1");
			exit(EXIT_FAILURE);
		}
	}
	if(nread<0){
		fprintf(stderr, "Errore read")
		exit(EXIT_FAILURE);
	}
	close(sd);

	return 0;
}

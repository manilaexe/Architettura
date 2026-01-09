//interfaccia: rstrlen hostname porta

#define _POSIX_C_SOURCE 200112L
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include "utils.h"
#define SIZE 4096

int main(int argc, char *argv[]){

	if(argc!=3){
		fprintf(stderr, "Uso: rstrlen hostname porta");
		exit(EXIT_FAILURE);
	}
	int sd, err;
	struct addrinfo hints, *res, *ptr;
	char s1[SIZE], buffer[SIZE];
	size_t nread;


	memset(&hints, 0, sizeof(hints));
	hints.ai_family=AF_UNSPEC;
	hints.ai_socktype=SOCK_STREAM;

	err=getaddrinfo(argv[1], argv[2], &hints, &res);
	if(err!=0){
		fprintf(stderr, "Errore gai");
		exit(EXIT_FAILURE);		
	}

	for(ptr=res; ptr!=NULL; ptr=ptr.ai_next){
		sd=socket(ptr->ai_family, prt->ai_socktype, ptr->ai_protocol);
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

	while(1){
		printf("Inserire una stringa per contarne i caratteri: ");
		scanf("%s",s1);
		if(strcmp(s1,"fine")==0){
			break;
		}
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
	}

	while((nread=read(sd, buffer, sizeof(buffer)))>0){
		err=write(1, buffer, sizeof(buffer));
		if(err<0){
			fprintf(stderr, "Errore stringa1");
			exit(EXIT_FAILURE);
		}
	}
	if(nread<0){
		fprintf(stderr, "Errore read");
		exit(EXIT_FAILURE);	
	}

	close(sd);

	return 0;
}

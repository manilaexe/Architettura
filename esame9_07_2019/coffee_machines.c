//interfaccia: coffee_machines server    porta
//              argv[0]         argv[1]  argv[2]

#define _POSIX_C_SOURCE 200112L
#define DIM 80
#define MAX_REQUEST_SIZE 4096
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rxb.h"
#include <netdb.h>
#include <unistd.h>

int main(int argc, char *argv[]){

    //controllo degli argomenti
    if(argc!=3){
        fprintf(stderr, "Uso: coffee_machines server porta");
        exit(EXIT_FAILURE);
    }

    int err, sd;
    struct addrinfo hints, *res, *ptr;
    char line[4096];
    rxb_t rxb;

    //chiamata DNS
    memset(&hints, 0, sizeof(hints));
    hints.ai_family=AF_UNSPEC;
    hints.ai_socktype=SOCK_STREAM;
    
    err=getaddrinfo(argv[1], argv[2], &hints, &res);
    if(err!=0){
        fprintf(stderr, "Errore gai");
        exit(EXIT_FAILURE);        
    }

    //procedura di connessione
    for(ptr=res; ptr!=NULL; ptr=ptr->ai_next){ //connesione con fallback
        sd=socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if(sd<0){
            continue;
        }
        err=connect(sd, ptr->ai_addr, ptr->ai_addrlen);
        if(err==0){ //connect va bene
            break;
        }
        close(sd);
    }
    if(ptr==NULL){
        fprintf(stderr, "Errore");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(res);

    rxb_init(&rxb, MAX_REQUEST_SIZE*2);

    //ciclo di servizio
    while(1){
        char username[DIM], password[DIM], categoria[DIM   ];

        puts("Inserisci username: ");
        if(fgets(username, sizeof(username), stdin)==NULL){
            fprintf(stderr, "Errore");
            exit(EXIT_FAILURE);        
        }

        puts("Inserisci password: ");
        if(fgets(password, sizeof(password), stdin)==NULL){
            fprintf(stderr, "Errore");
            exit(EXIT_FAILURE);        
        }

        puts("Inserisci categoria della macchina: ");
        if(fgets(categoria, sizeof(categoria), stdin)==NULL){
            fprintf(stderr, "Errore");
            exit(EXIT_FAILURE);        
        }        
        
        //write
        if(write_all(sd, username, strlen(username))<0){
            fprintf(stderr, "Errore");
            exit(EXIT_FAILURE);        
        }

        if(write_all(sd, password, strlen(password))<0){
            fprintf(stderr, "Errore");
            exit(EXIT_FAILURE);        
        }

        if(write_all(sd, categoria, strlen(categoria))<0){
            fprintf(stderr, "Errore");
            exit(EXIT_FAILURE);        
        }

        //lettura risposta del server
        while(1){
            size_t buflen=sizeof(line-1);
            memset(line, 0, sizeof(line));

            err=rxb_readline(&rxb, sd, line, &buflen);  
            if(err==0){ //input terminato
                close(sd);
                exit(EXIT_SUCCESS);              
            }            
            if(err<0){
                fprintf(stderr, "Errore");
                exit(EXIT_FAILURE);              
            }

            if(strcmp(line, "--- END REQUEST ---")==0){
                break;
            }
            puts(line);
            /*OPPURE
            printf("%s\n", line);
            OPPURE
            write_all(STDOUT_FINLENO, line, strlen(line));
            write(STDOUT_FILENO, "\n", 1);           
            */
        }

    }

    rxb_destroy(&rxb);
    close(sd);

    return 0;
}

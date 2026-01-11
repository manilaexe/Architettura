//Interfaccia: esamina_spese_principali server    porta
//             argv[0]                  argv[1]   argv[2]
//input: mese (YYYYMM), categoria di spesa, numero di spese di interesse
#define _POSIX_C_SOURCE 200809L
#define MAX_REQUEST 4096
#define DIM 40

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rxb.h"
#include <netdb.h>
#include <unistd.h>

int main(int argc, char *argv[]){

    if(argc!=3){
        fprintf(stderr, "Uso: esamina_spese_principali server porta");
        exit(EXIT_FAILURE);
    }

    int err, sd;
    struct addrinfo hints, *res, *ptr;
    char line[4096];
    rxb_t rxb;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family=AF_UNSPEC;
    hints.ai_socktype=SOCK_STREAM;

    err=getaddrinfo(argv[1], argv[2], &hints, &res);
    if(err!=0){
        fprintf(stderr, "Errore");
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
        fprintf(stderr, "Errore");
        exit(EXIT_FAILURE);   
    }
    freeaddrinfo(res);

    rxb_init(&rxb, MAX_REQUEST*2);

    while(1){
        char mese[DIM], cat[DIM], bufn[DIM];
        int n;

        puts("Inserire il mese di interesse (fomrato YYYYMM)");
        if(fgets(mese, sizeof(mese), stdin)==NULL){
            fprintf(stderr, "Errore");
            exit(EXIT_FAILURE);              
        }
        puts("Inserire la categoria di interesse");
        if(fgets(cat, sizeof(cat), stdin)==NULL){
            fprintf(stderr, "Errore");
            exit(EXIT_FAILURE);              
        }
        puts("Inserire il numero di spese da visualizzare");
        if(fgets(bufn, sizeof(bufn), stdin)==NULL){
            fprintf(stderr, "Errore");
            exit(EXIT_FAILURE);              
        }
        n=atoi(bufn); //converto in numero

        if(write_all(sd, mese, strlen(mese))<0){
            fprintf(stderr, "Errore");
            exit(EXIT_FAILURE);              
        }
        if(write_all(sd, cat, strlen(cat))<0){
            fprintf(stderr, "Errore");
            exit(EXIT_FAILURE);              
        }
        if(write_all(sd, bufn, strlen(bufn))<0){
            fprintf(stderr, "Errore");
            exit(EXIT_FAILURE);              
        }      
        
        while(1){
            size_t buflen=sizeof(line)-1;
            memset(line, 0, sizeof(line));

            err=rxb_readline(&rxb,sd,line,&buflen);
            if(err==0){                      
                close(sd);
                exit(EXIT_SUCCESS);             
            }
            if(err<0){
                fprintf(stderr, "Errore");
                exit(EXIT_FAILURE);                 
            }
            if(strcmp(line, "--- END REQUEST ---\n")==0){
                break;
            }
            puts(line);
        }

    }
    rxb_destroy(&rxb);
    close(sd);

    return 0;
}

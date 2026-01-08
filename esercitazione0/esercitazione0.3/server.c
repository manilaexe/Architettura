//Interfaccia: server  porta    
//             argv[0] argv[1]  
#define _POSIX_C_SOURCE 200112L
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include "rxb.h"
#define MAX_REQUEST_SIZE 4096
int main(int argc, char *argv[]){
    int sd, err;
    struct addrinfo hints, *res;
    //controllo argomenti
    if(argc!=2){
        fprintf(stderr, "Uso: Server porta");
        exit(EXIT_FAILURE);
    }

    memset(&hints, 0, sizeof(hints)); //direttive getaddrinfo
    hints.ai_family=AF_UNSPEC; //ok sia ipv6 che ipv4
    hints.ai_socktype=SOCK_STREAM; //mi interessano solo le socket di tipo stream
    hints.ai_flags=AI_PASSIVE;  //usa la getaddrinfo in maniera passiva
                                // voglio solo la struttura da passare a bind

    //connessione
    err=getaddrinfo(NULL, argv[1], &hints, &res); //sockaddr pronta per il bind sulla porta
    if(err!=0){
        fprintf(stderr, "Errore gai: %s\n", gai_strerror(err));
        exit(EXIT_FAILURE);
    }

    sd=socket(res->ai_family, res->ai_socktype, res->ai_protocol); //crea la socket TCP
    if(sd<0){
        fprintf(stderr, "Errore socket");
        exit(EXIT_FAILURE);
    }

    err=bind(sd, res->ai_addr, res->ai_addrlen); //associa socket, indirizzo e porta
    if(err<0){
        fprintf(stderr, "Errore connessione");
        exit(EXIT_FAILURE);
    }
    
    freeaddrinfo(res); //libera la memoria di getaddrinfo

    err=listen(sd, SOMAXCONN); //trasforma la socket in passiva di ascolto
                               //SOMAXCONN=massimo backlog consentito dal sistema
     if(err<0){
        fprintf(stderr, "Errore listen");
        exit(EXIT_FAILURE);
    }

    while(1){
        int ns, pid;
        rxb_t buf;
        size_t len;
        char nomefile[MAX_REQUEST_SIZE*2];

        ns=accept(sd, NULL, NULL); //accept(socket di ascolto, socket attiva)
        if(ns<0){
            fprintf(stderr, "Errore accept");
            exit(EXIT_FAILURE);
        }
       
        //gestione della richiesta di servizio 
        //inizializzazione del buffer
        rxb_init(&buf, MAX_REQUEST_SIZE*2); //alloca dinamicamente il buffer interno
        //readline
        len=sizeof(nomefile);
        err=rxb_readline(&buf, ns, nomefile, &len); //devo usare la socket attiva
                                                    //read line robusta
        if(err<0){
            fprintf(stderr, "Errore listen");
            exit(EXIT_FAILURE);
        }

        //fork e head
        pid=fork();
        if(pid<0){
            fprintf(stderr, "Errore fork");
            exit(EXIT_FAILURE);        
        }else if(pid==0){ //sono nel figlio che gestisce la richiesta
            //redirezione su socket
            close(1);
            dup(ns);
            err=dup(ns);
            if(err<0){
                fprintf(stderr, "Errore dup");
                exit(EXIT_FAILURE);
            }
            close(ns);
            execlp("head", "head", "-n", "5", nomefile, NULL);
            fprintf(stderr, "Errore exec");
            exit(EXIT_FAILURE);
        }
        rxb_destroy(&buf); //pulico il buffer 
        //sono nel padre che resta in ascolto
        close(ns);
    }
    close(sd);


    return 0;
}

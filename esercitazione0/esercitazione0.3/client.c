//Interfaccia: java RemoteHeadclient hostname porta    nomefile
//                  argv[0]          argv[1]  argv[2]  argv[3]
#define _POSIX_C_SOURCE 200112L //usa API POSIX fino allo standard del 2001
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include "rxb.h"   

int main(int argc, char **argv){

    int sd, err, nread; //nread=byte letti con read
    struct addrinfo hints, *res, *ptr;  //hints=criteri di ricerca per getaddrinfo
                                        //res=testa della lista di indirizzi trovati
                                        //ptr=puntatore per scorrere la lista
    uint8_t buffer[4096]; //buffer di lettura

    //controllo argomenti
    if(argc!=4){
        fprintf(stderr, "Uso: RemoteHeadClient hostname porta nomefile");
        exit(EXIT_FAILURE);
    }

    memset(&hints, 0, sizeof(hints)); //direttive getaddrinfo
    hints.ai_family=AF_UNSPEC; //ok sia IPv6 che IPv4
    hints.ai_socktype=SOCK_STREAM; //mi interessano solo le socket di tipo stream (TCP)

    //connessione
    err=getaddrinfo(argv[1], argv[2], &hints, &res); //risoluzione dell'indirizzo
    //gestione errori di indirizzo
    if(err!=0){
        fprintf(stderr, "Errore gai: %s\n", gai_strerror(err));
        exit(EXIT_FAILURE);
    }
    
    //tentativo di connessione
    for(ptr=res; ptr!=NULL; ptr->ai_next){ //scorre tutti gli indirizzi possibili
        sd=socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol); //crea una socket con i parametri di getaddrinfo
        if(sd<0){ //se fallisce passa all'indirizzo successivo
            continue;
        }
        err=connect(sd, ptr->ai_addr, ptr->ai_addrlen); //tenta la connessione TCP
        if(err==0){ //se funziona esce dal ciclo
            break;
        }
        close(sd);//se fallisce chiuda la socket e riprova
    }
    //se nessun indirizzo funziona da errore
    if(ptr==NULL){
        fprintf(stderr, "Errore connessione");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(res); //libera la memoria usata da getaddrinfo

    //scrivo il nome del file in UTF-8
    err=write_all(sd, argv[3], strlen(argv[3])); //write potrebbe scrivere meno byte, write_all no
    if(err<0){
        fprintf(stderr, "Errore write");
        exit(EXIT_FAILURE);
    }
    //scrivo anche il terminatore
    err=write(sd, "\n", 1); 
    if(err<0){
        fprintf(stderr, "Errore write");
        exit(EXIT_FAILURE);
    }
    //lettura della risposta
    while((nread=read(sd, buffer, sizeof(buffer)))>0){ //legge dal server finche` non chiude la connessione
        err=write_all(1, buffer, nread);//scrivo sullo stdout
                                        //raccoglie i dati nel buffer fino a sizeof(buffer)
        if(err<0){
            fprintf(stderr, "Errore write");
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

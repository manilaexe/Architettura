//Interfaccia: java RemoteHeadclient hostname porta    nomefile
//                  argv[0]          argv[1]  argv[2]  argv[3]
#define _POSIX_C_SOURCE 200112L
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>

int main(int argc, char **argv){

    int err;
    struct addrinfo hints, *res, *ptr;

    //controllo argomenti
    if(argc!=4){
        fprintf(stderr, "Uso: RemoteHeadClient hostname porta nomefile");
        exit(EXIT_FAILURE);
    }

    memset(&hints, 0, sizeof(hints)); //direttive getaddrinfo
    hints.ai_family=AF_UNSPEC; //ok sia ipv6 che ipv4
    hints.ai_socktype=SOCK_STREAM; //mi interessano solo le socket di tipo stream
    err=getaddrinfo(argv[1], argv[2], &hints, &res);
    if(err!=0){
        fprintf(stderr, "Errore gai: %s\n", gai_stderror(err));
        exit(EXIT_FAILURE);
    }
    
    for(ptr=res; ptr!=NULL; ptr->ai_next){
        sd=socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if(sd<0){
            continue;
        }
        err=connect(sd, ptr->ai_addr, ptr->addrlen);
        if(err==0){
            break;
        }
        close(sd);
    }



    return 0;
}

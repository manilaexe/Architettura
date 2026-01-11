//Interfaccia: server  porta
//             argv[0] argv[1]  
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
#include <signal.h>
#include <errno.h>


void handler(int signo){
    int status;
    (void) signo;
    while(waitpid(-1, &status, WNOHANG)>0){
        continue;
    }
}

int main(int argc, char *argv[]){

    if(argc!=2){
        fprintf(stderr, "Uso: server porta");
        exit(EXIT_FAILURE);
    }

    int err, sd, on;
    struct addrinfo hints, *res;
    struct sigaction sa={0};

    sigemptyset(&sa.sa_mask);
    sa.sa_flags=SA_RESTART;
    sa.sa_handler=handler;

    if(sigaction(SIGCHLD, &sa, NULL)==-1){
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family=AF_UNSPEC;
    hints.ai_socktype=SOCK_STREAM;
    hints.ai_flags=AI_PASSIVE;

    err=getaddrinfo(NULL, argv[1], &hints, &res);
    if(err!=0){
        fprintf(stderr, "Errore");
        exit(EXIT_FAILURE);        
    }

    sd=socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if(sd<0){
        fprintf(stderr, "Errore");
        exit(EXIT_FAILURE);        
    }

    on=1;
    if(setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))<0){
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    err=bind(sd, res->ai_addr, res->ai_addrlen);
    if(err<0){
        fprintf(stderr, "Errore");
        exit(EXIT_FAILURE);        
    }

    err=listen(sd, SOMAXCONN);
    if(err<0){
        fprintf(stderr, "Errore");
        exit(EXIT_FAILURE);        
    }

    while(1){
        int ns, pid_f;
        ns=accept(sd, NULL, NULL);
        if(ns<0){
            if(errno==EINTR){
                continue;
            }else{
                fprintf(stderr, "Errore");
                exit(EXIT_FAILURE);  
            }
        }
        pid_f=fork();
        if(pid_f<0){
            fprintf(stderr, "Errore");
            exit(EXIT_FAILURE);  
        }else if(pid_f==0){//figlio
            rxb_t rxb;
            char mese[DIM], cat[DIM], bufn[DIM];
            int n;
            size_t mese_len, cat_len, bufn_len;
            close(sd);

            rxb_init(&rxb, MAX_REQUEST*2);

            while(1){
                char *end="--- END REQUEST ---\n";
                int pid_cut, pid_sort, pid_head, status, p1p2[2];
                memset(mese, 0, sizeof(mese));
                mese_len=sizeof(mese)-1;
                err=rxb_readline(&rxb, ns, mese, &mese_len);
                if(err<0){
                    fprintf(stderr, "Errore");
                    exit(EXIT_FAILURE);        
                }   
                memset(cat, 0, sizeof(cat));
                cat_len=sizeof(cat)-1;
                err=rxb_readline(&rxb, ns, cat, &cat_len);
                if(err<0){
                    fprintf(stderr, "Errore");
                    exit(EXIT_FAILURE);        
                }                 
                memset(bufn, 0, sizeof(bufn));
                bufn_len=sizeof(bufn)-1;
                err=rxb_readline(&rxb, ns, bufn, &bufn_len);
                if(err<0){
                    fprintf(stderr, "Errore");
                    exit(EXIT_FAILURE);        
                }         
                
                signal(SIGCHLD, SIG_DFL);
                if(pipe(p1p2)<0){
                    fprintf(stderr, "Errore");
                    exit(EXIT_FAILURE); 
                }
                pid_sort=fork();
                if(pid_sort<0){
                    fprintf(stderr, "Errore");
                    exit(EXIT_FAILURE); 
                }else if(pid_sort==0){//nipote sort
                    char filename[MAX_REQUEST];
                    close(ns);
                    close(1);
                    dup(p1p2[1]);
                    close(p1p2[1]);
                    close(p1p2[0]);
                    //snprintf(filename, sizeof(filename), "/var/local/expenses/%s/%s.txt", mese, cat);
                    snprintf(filename, sizeof(filename), "/%s/%s.txt", mese, cat);
                    execlp("sort", "sort", "-r", "-n", filename, NULL);
                    perror("errore execl sort");
                    exit(EXIT_FAILURE);
                }

                pid_head=fork();
                if(pid_head<0){
                    fprintf(stderr, "Errore");
                    exit(EXIT_FAILURE); 
                }else if(pid_head == 0){
                    close(0);
                    dup(p1p2[0]);
                    close(p1p2[0]);
                    close(p1p2[1]);
                    close(1);
                    dup(ns);
                    close(ns);
                    execlp("head", "head", "-n", bufn, NULL);
                    perror("errore execl head");
                    exit(EXIT_FAILURE);                    
                }

                close(p1p2[0]);
                close(p1p2[1]);
                waitpid(pid_head, &status, 0);
                waitpid(pid_sort, &status, 0);
                
                write_all(ns, end, strlen(end));                           
            }
            close(ns);
            rxb_destroy(&rxb);
            exit(EXIT_SUCCESS);    
        }//padre
        close(ns);
    }
    close(sd);
    return 0;
}

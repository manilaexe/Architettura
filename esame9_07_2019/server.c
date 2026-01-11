    #define _POSIX_C_SOURCE 200809L
    #define DIM 80
    #define MAX_REQUEST_SIZE 4096
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include "rxb.h"
    #include <netdb.h>
    #include <unistd.h>
    #include <sys/wait.h>
    #include <signal.h>
    #include <sys/socket.h>
    #include <errno.h>

    int autorizza(const char *username, const char *password){
        return 1;
    }
    //implementazione del SIGCHILD
    void handler(int signo){
        int status;
        (void) signo;
        while(waitpid(-1, &status, WNOHANG)>0){
            continue;
        }
    }


    int main(int argc, char *argv[]){

        if(argc!=2){
            fprintf(stderr, "Uso:server porta");
            exit(EXIT_FAILURE);        
        }

        int err, sd, on; 
        struct addrinfo hints, *res;

        struct  sigaction sa={0};

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

        //opzionale ma utile perche` cosi` e` possibile rilanciare il server sulla stessa socket
        on=1;
        if(setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))<0){
            perror("setsockopt");
            exit(EXIT_FAILURE);
        }

        //bind
        err=bind(sd, res->ai_addr, res->ai_addrlen);
        if(err<0){
            fprintf(stderr, "Errore");
            exit(EXIT_FAILURE);
        }

        //listen
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
            }else if(pid_f==0){ //figlio
                rxb_t rxb;
                char username[DIM], password[DIM], categoria[DIM];
                size_t username_len, password_len, categoria_len;
                close(sd); //chiudo socket passiva
                
                //creo buvver rxb
                rxb_init(&rxb, MAX_REQUEST_SIZE*2);

                //leggere 3 info
                while(1){
                    char *end="--- END REQUEST ----\n";
                    int pid_cut, pid_sort, pid_head, status;
                    memset(username, 0, sizeof(username));
                    username_len=sizeof(username)-1; //-1 per il terminatore
                    err=rxb_readline(&rxb, ns, username, &username_len);
                    if(err<0){
                        fprintf(stderr, "Errore");
                        exit(EXIT_FAILURE);
                    }

                    memset(password, 0, sizeof(password));
                    password_len=sizeof(password)-1; //-1 per il terminatore
                    err=rxb_readline(&rxb, ns, password, &password_len);
                    if(err<0){
                        fprintf(stderr, "Errore");
                        exit(EXIT_FAILURE);
                    }

                    memset(categoria, 0, sizeof(categoria));
                    categoria_len=sizeof(categoria)-1; //-1 per il terminatore
                    err=rxb_readline(&rxb, ns, categoria_len, &categoria_len);
                    if(err<0){
                        fprintf(stderr, "Errore");
                        exit(EXIT_FAILURE);
                    }
                    //chiamare autorizza
                    if(autorizza(username, password)!=1){
                        char *unauth="non autorizzato\n";
                        write_all(ns, unauth, strlen(unauth));
                        write_all(ns, end, strlen(end));
                        continue;
                    }

                    //se ok 3 fork -> creo nipoti
                    signal(SIGCHLD, SIG_DFL);
                    pid_cut=fork();
                    pid_sort=fork();
                    pid_head=fork();
                    wait(pid(pid_cut, &status, 0));
                    wait(pid(pid_sort, &status, 0));
                    wait(pid(pid_head, &status, 0));
                    
                    write_all(ns, end, strlen(end));

                }



                close(ns);
                rxb_destroy(&rxb);
                exit(EXIT_SUCCESS);

            }
            //padre
            close(ns);
        }

        close(sd);
        
        return 0;
    }

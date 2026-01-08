#define _POSIX_C_SOURCE 200809L
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv)
{
        char buff[2048];
        int sd, err, nread;
        struct addrinfo hints, *ptr, *res;

        if (argc != 5) {
                fprintf(stderr, "Usage: %s hostname port string1 string2\n", argv[0]);
                exit(EXIT_FAILURE);
        }

	/* ignore SIGPIPE */
	signal(SIGPIPE, SIG_IGN);

        memset(&hints, 0, sizeof(hints));
        hints.ai_family   = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;

        err = getaddrinfo(argv[1], argv[2], &hints, &res);
        if (err != 0) {
                fprintf(stderr, "Error on getaddrinfo: %s\n", gai_strerror(err));
                exit(EXIT_FAILURE);
        }

        /* connection with fallback */
        for (ptr=res; ptr != NULL; ptr=ptr->ai_next) {
                sd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
                /* if socket creation fails, try next address */
                if (sd < 0)
                        continue;

                /* if connect is successful, exit loop */
                if (connect(sd, ptr->ai_addr, ptr->ai_addrlen) == 0)
                        break;

                /* otherwise, close socket and try next address */
                close(sd);
        }

        /* control client connection */
        if (ptr == NULL) {
                fprintf(stderr, "Connection error!\n");
                exit(EXIT_FAILURE);
        }

        /* I can free the memory allocated by getaddrinfo */
        freeaddrinfo(res);

        /* Sending first string. No terminator 
        --> attention on the server side */
        if (write(sd, argv[3], strlen(argv[3])) < 0) {
                perror("write");
                exit(EXIT_FAILURE);
        }

        /* Waiting for ACK from Server */
        if (read(sd, buff, sizeof(buff)) < 0) {
                perror("read");
                exit(EXIT_FAILURE);
        }

        /* Sending second string. No terminator 
        --> attention on the server side */
        if (write(sd, argv[4], strlen(argv[4])) < 0) {
                perror("write");
                exit(EXIT_FAILURE);
        }

        /* Receiving result */
        while ((nread = read(sd, buff, sizeof(buff))) > 0) {
                if (write(1, buff, nread) < 0) {
                        perror("write on stdout");
                        exit(EXIT_FAILURE);
                }
        }

        /* Control and error checking */
        if (nread < 0) {
                perror("read result");
                exit(EXIT_FAILURE);
        }

        /* print \n before termination */
        if (write(1, "\n", 1) < 0) {
                perror("write");
                exit(EXIT_FAILURE);
        }

        /* closing the socket */
        close(sd);

        return 0;
}


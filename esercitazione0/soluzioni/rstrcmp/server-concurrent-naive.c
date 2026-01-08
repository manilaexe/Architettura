#define _POSIX_C_SOURCE 200809L
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


/* SIGCHLD handler */
void handler(int signo)
{
        int status;

        (void)signo; /* avoid warning */

        /* non-blocking wait for terminated children */
        while (waitpid(-1, &status, WNOHANG) > 0)
                continue;
}


int main(int argc, char **argv)
{
        int sd, err, on;
        struct addrinfo hints, *res;
        struct sigaction sa;

	/* Parameters checking */
        if (argc != 2) {
                fprintf(stderr, "Usage: %s port\n", argv[0]);
                exit(EXIT_FAILURE);
        }

	/* Ignore SIGPIPE */
	signal(SIGPIPE, SIG_IGN);

	/* Install SIGCHLD handler using sigaction syscall, which is a
	 * POSIX standard, instead of signal which has different semantics
	 * depending on the operating system */
        sigemptyset(&sa.sa_mask);
        /* use SA_RESTART to avoid having to explicitly check if
         * accept was interrupted by a signal and in that case restart it
         * (see paragraph 21.5 of the text M. Kerrisk, "The Linux
         * Programming Interface") */
        sa.sa_flags   = SA_RESTART;
        sa.sa_handler = handler;

        if (sigaction(SIGCHLD, &sa, NULL) == -1) {
                perror("sigaction");
                exit(EXIT_FAILURE);
        }

	/* Prepare getaddrinfo */
        memset(&hints, 0, sizeof(hints));
        /* Use AF_INET to force only IPv4, AF_INET6 to force only IPv6 */
        hints.ai_family   = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags    = AI_PASSIVE;

	/* Use getaddrinfo to prepare data structures to use with socket and bind */
        if ((err = getaddrinfo(NULL, argv[1], &hints, &res)) != 0) {
                fprintf(stderr, "Error setting up bind address: %s\n", gai_strerror(err));
                exit(EXIT_FAILURE);
        }

	/* Create socket */
        if ((sd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0){
                perror("Error creating socket");
                exit(EXIT_FAILURE);
        }

	/* Disable waiting for TIME_WAIT phase before socket creation 
        * When a server process closes a socket,
        * the TCP port remains in the TIME_WAIT
        * state for a certain period to prevent issues
        * with delayed packets. */
        on = 1;
        if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0){
                perror("setsockopt");
                exit(EXIT_FAILURE);
        }

	/* Bind the socket to the desired port */
        if (bind(sd, res->ai_addr, res->ai_addrlen) < 0) {
                perror("Error binding socket");
                exit(EXIT_FAILURE);
        }

        /* I can free the memory allocated by getaddrinfo */
        freeaddrinfo(res);

        /* Transform into passive listening socket */
        /* SOMAXCONN: maximum length of the queue of pending 
        connections */
        if (listen(sd, SOMAXCONN) < 0) {
                perror("listen");
                exit(EXIT_FAILURE);
        }

        for(;;) {
                int ns, pid, nread;

		/* I wait for connection requests */
                if ((ns = accept(sd, NULL, NULL)) < 0) {
                        perror("accept");
                        exit(EXIT_FAILURE);
                }

		/* Create a child process to handle the request */
                if ((pid = fork()) < 0) {
                        perror("fork");
                        exit(EXIT_FAILURE);
                } else if (pid == 0) { /* CHILD */
                        char str1[4096], str2[4096], response[80];
                        const char *ack = "ACK";

                        /* Close the passive socket */
                        close(sd);

                        /* Initialize the buffer to zero and read sizeof(str1)-1
                         * bytes, so I'm sure the buffer content will always be
                         * null-terminated. This way, I can interpret it as a C string
                         * and pass it directly to the strcmp function. */
                        memset(str1, 0, sizeof(str1));
                        if ((nread = read(ns, str1, sizeof(str1)-1)) < 0) {
                                perror("read");
                                exit(EXIT_FAILURE);
                        }

                        /* Send ACK to the client */
                        if (write(ns, ack, strlen(ack)) < 0) {
                                perror("write");
                                exit(EXIT_FAILURE);
                        }

                        /* Initialize the buffer to zero and read sizeof(str2)-1
                         * bytes, so I'm sure the buffer content will always be
                         * null-terminated. This way, I can interpret it as a C string
                         * and pass it directly to the strcmp function. */
                        memset(str2, 0, sizeof(str2));
                        if ((nread = read(ns, str2, sizeof(str2)-1)) < 0) {
                                perror("read");
                                exit(EXIT_FAILURE);
                        }

                        /* Prepare the response buffer */
                        if (strcmp(str1, str2) == 0) {
                                /* The strings are identical */
                                strncpy(response, "YES", sizeof(response));
                        } else {
                                /* The strings are different */
                                strncpy(response, "NO", sizeof(response));
			}

                        /* Send the response */
                        if (write(ns, response, strlen(response)) < 0) {
                                perror("write");
                                exit(EXIT_FAILURE);
                        }

                        /* Close the active socket */
                        close(ns);

                        /* Terminate the child */
                        exit(EXIT_SUCCESS);
                }

                /* PADRE */

                /* Chiudo la socket attiva */
                close(ns);
        }

        close(sd);

        return 0;
}


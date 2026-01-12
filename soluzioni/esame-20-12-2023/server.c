#define _POSIX_C_SOURCE 200809L
#include <limits.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "rxb.h"
#include "utils.h"

#define MAX_LINE_LENGTH 1024

void sigchld_handler(int signo)
{
	int status;

	(void)signo;

        /* Deallocazione risorse di TUTTI i figli terminati (con wait
         * deallocheremmo solo le risorse del primo) */
        while(waitpid(-1, &status, WNOHANG) > 0)
		continue;
}

/* server-concurrent-td-connreuse port 
 *                                argv[1] */
int main(int argc, char *argv[])
{
	int err, sd, on;
	struct addrinfo hints, *res;
	struct sigaction sa;

	/* Controllo numero argomenti */
	if (argc != 2) {
		fprintf(stderr, "Usage: %s port\n", argv[0]);
		return EXIT_FAILURE;
	}

	/* Ignoro SIGPIPE */
	/* signal(SIGPIPE, SIG_IGN); */
	err = sigaction(SIGPIPE, NULL, NULL);
	if (err < 0) {
		perror("sigaction SIGPIPE");
		return EXIT_FAILURE;
	}

	/* Installo gestore SIGCHLD */
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = sigchld_handler;
	sa.sa_flags = SA_RESTART;
	sigemptyset(&sa.sa_mask);

	err = sigaction(SIGCHLD, &sa, NULL);
	if (err < 0) {
		perror("sigaction SIGCHLD");
		return EXIT_FAILURE;
	}

	/* Inizializzo hints */
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	/* Chiamo getaddrinfo */
	err = getaddrinfo(NULL, argv[1], &hints, &res);
	if (err != 0) {
		fprintf(stderr, "Errore in getaddrinfo: %s\n", gai_strerror(err));
		return EXIT_FAILURE;
	}

	/* Creo socket */
	sd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (sd < 0) {
		perror("socket");
		return EXIT_FAILURE;
	}

	/* Disabilito TIME_WAIT */
	on = 1;
	err = setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	if (err < 0) {
		perror("setsockopt");
		return EXIT_FAILURE;
	}

	/* Chiamo bind */
	err = bind(sd, res->ai_addr, res->ai_addrlen);
	if (err < 0) {
		perror("bind");
		return EXIT_FAILURE;
	}

	freeaddrinfo(res);

	/* Chiamo listen */
	err = listen(sd, SOMAXCONN);
	if (err < 0) {
		perror("listen");
		return EXIT_FAILURE;
	}

	for(;;) {
		/* Chiamo accept */
		int ns = accept(sd, NULL, NULL);
		if (ns < 0) {
			perror("accept");
			return EXIT_FAILURE;
		}

		pid_t pid_child = fork();
		if (pid_child < 0) {
			perror("fork");
			return EXIT_FAILURE;
		} else if (pid_child == 0) { /* FIGLIO */
			rxb_t rbx;

			/* Chiudo socket passiva */
			close(sd);

			/* Inizializzo buffer rxb_readline */
			rxb_init(&rbx, 2 * MAX_LINE_LENGTH);

			/* Reinstallo gestore default SIGCHLD */
			memset(&sa, 0, sizeof(sa));
			sa.sa_handler = SIG_DFL;
			sa.sa_flags = SA_RESTART;
			sigemptyset(&sa.sa_mask);

			err = sigaction(SIGCHLD, &sa, NULL);
			if (err < 0) {
				perror("sigaction SIGCHLD");
				return EXIT_FAILURE;
			}

			/* Request loop */
			for (;;) {
				char mese[MAX_LINE_LENGTH + 1];
				char tipologia[MAX_LINE_LENGTH + 1];
				char località[MAX_LINE_LENGTH + 1];
				size_t mese_len, tipologia_len, località_len;
				char filename[PATH_MAX+1];
				pid_t pid_n1, pid_n2;
				int pipe_n1n2[2];

				/* Inizializzo mese */
				memset(mese, 0, sizeof(mese));
				mese_len = sizeof(mese) - 1;

				/* Leggo mese */
				if (rxb_readline(&rbx, ns, mese, &mese_len) < 0) {
					perror("rxb_readline mese");
					return EXIT_FAILURE;
				}

#ifdef USE_LIBUNISTRING
				/* Controllo UTF-8 */
				if (u8_check((uint8_t *)mese, mese_len) != NULL) {
					fprintf(stderr, "Errore: stringa mese non testo UTF-8 valido\n");
					return EXIT_FAILURE;
				}
#endif

				/* Inizializzo tipologia */
				memset(tipologia, 0, sizeof(tipologia));
				tipologia_len = sizeof(tipologia) - 1;

				/* Leggo tipologia */
				if (rxb_readline(&rbx, ns, tipologia, &tipologia_len) < 0) {
					perror("rxb_readline tipologia");
					return EXIT_FAILURE;
				}

#ifdef USE_LIBUNISTRING
				/* Controllo UTF-8 */
				if (u8_check((uint8_t *)tipologia, tipologia_len) != NULL) {
					fprintf(stderr, "Errore: stringa tipologia non testo UTF-8 valido\n");
					return EXIT_FAILURE;
				}
#endif

				/* Inizializzo località */
				memset(località, 0, sizeof(località));
				località_len = sizeof(località) - 1;

				/* Leggo località */
				if (rxb_readline(&rbx, ns, località, &località_len) < 0) {
					perror("rxb_readline località");
					return EXIT_FAILURE;
				}

#ifdef USE_LIBUNISTRING
				/* Controllo UTF-8 */
				if (u8_check((uint8_t *)località, località_len) != NULL) {
					fprintf(stderr, "Errore: stringa località non testo UTF-8 valido\n");
					return EXIT_FAILURE;
				}
#endif

				/* Predispongo la stringa filename */
                                snprintf(filename, sizeof(filename),
                                         "./holidays/%s/%s.txt",
                                         /* "/var/local/holidays/%s/%s.txt", */
                                         tipologia, mese);
				
				if (pipe(pipe_n1n2) < 0) {
					perror("pipe");
					return EXIT_FAILURE;
				}

				pid_n1 = fork();
				if (pid_n1 < 0) {
					perror("fork");
					return EXIT_FAILURE;
				} else if (pid_n1 == 0) {
					/* NIPOTE 1 */

					/* grep località filename */

					/* Redirezione output su pipe */
					close(1);
					dup(pipe_n1n2[1]);

					/* Chiusura file descriptor non utilizzati */
					close(pipe_n1n2[0]);
					close(pipe_n1n2[1]);
					close(ns);

					execlp("grep", "grep", località, filename, (char *)NULL);
					perror("execlp grep");
					exit(EXIT_FAILURE);
				}

				pid_n2 = fork();
				if (pid_n2 < 0) {
					perror("fork");
					return EXIT_FAILURE;
				} else if (pid_n2 == 0) {
					/* NIPOTE 2 */

					/* sort -r -n */

					/* Redirigo input da pipe */
					close(0);
					dup(pipe_n1n2[0]);

					/* Redirigo output su socket */
					close(1);
					dup(ns);

					/* Chiusura file descriptor non utilizzati */
					close(pipe_n1n2[0]);
					close(pipe_n1n2[1]);
					close(ns);

					execlp("sort", "sort", "-r", "-n", (char *)NULL);
					perror("execlp sort");
					exit(EXIT_FAILURE);
				}

				/* FIGLIO */
				char *stringa_terminazione = "---END RESPONSE---\n";

				/* Chiusura file descriptor non utilizzati */
				close(pipe_n1n2[0]);
				close(pipe_n1n2[1]);

				/* Attesa terminazione nipoti */
				waitpid(pid_n1, NULL, 0);
				waitpid(pid_n2, NULL, 0);

				/* Scrivo messaggio fine risposta */
				err = write_all(ns, stringa_terminazione, 
						strlen(stringa_terminazione));
				if (err < 0) {
					perror("write_all");
					return EXIT_FAILURE;
				}
			}

			/* Deinizializzo rxb */
			rxb_destroy(&rbx);
			
			/* Chiusura socket attiva */
			close(ns);

			return EXIT_SUCCESS;
		}

		/* PADRE */

		/* Chiusura socket attiva */
		close(ns);
	}

	return EXIT_SUCCESS;
}

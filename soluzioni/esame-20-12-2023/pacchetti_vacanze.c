#define _POSIX_C_SOURCE 200809L
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef USE_LIBUNISTRING
#include <unistr.h>
#endif
#include "utils.h"
#include "rxb.h"

#define MAX_LINE_LENGTH 1024

/* client-td-connreuse hostname porta 
 *                     argv[1]  argv[2] */
int main(int argc, char *argv[])
{
	int err, sd;
	struct addrinfo hints, *res, *ptr;
	rxb_t rxb;

	/* Ignoro SIGPIPE per evitare crash in caso di scrittura su socket chiusa */
	signal(SIGPIPE, SIG_IGN);

	/* Controllo errori */
	if (argc != 3) {
		fprintf(stderr, "Uso: %s hostname porta\n", argv[0]);
		return EXIT_FAILURE;
	}

	/* Inizializzazione hints */
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	/* Connessione al server */
	err = getaddrinfo(argv[1], argv[2], &hints, &res);
	if (err != 0) {
		fprintf(stderr, "Errore in getaddrinfo: %s\n", gai_strerror(err));
		return EXIT_FAILURE;
	}

	/* Connessione con fallback */
	for (ptr = res; ptr != NULL; ptr = ptr->ai_next) {
		sd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (sd < 0) {
			continue;
		}

		err = connect(sd, ptr->ai_addr, ptr->ai_addrlen);
		if (err == 0) {
			printf("Connessione riuscita\n");
			break;
		}

		close(sd);
	}

	/* Verifico se connessione è riuscita */
	if (ptr == NULL) {
		fprintf(stderr, "Errore: nessuna connessione riuscita\n");
		return EXIT_FAILURE;
	}

	/* Deallocazione memoria */
	freeaddrinfo(res);

	/* Inizializzazione buffer interno readline */
	rxb_init(&rxb, 2 * MAX_LINE_LENGTH);

	for (;;) {
		/* Leggo mese, tipologia e località */
		char mese[1024], tipologia[1024], località[1024];

		printf("Inserisci mese (fine per terminare): ");

		if (fgets(mese, sizeof(mese), stdin) == NULL) {
			perror("fgets");
			return EXIT_FAILURE;
		}

		if (strcmp(mese, "fine\n") == 0) {
			break;
		}

		printf("Inserisci tipologia: ");

		if (fgets(tipologia, sizeof(tipologia), stdin) == NULL) {
			perror("fgets");
			return EXIT_FAILURE;
		}

		printf("Inserisci località: ");

		if (fgets(località, sizeof(località), stdin) == NULL) {
			perror("fgets");
			return EXIT_FAILURE;
		}

		if (write_all(sd, mese, strlen(mese)) < 0) {
			perror("write");
			return EXIT_FAILURE;
		}

		if (write_all(sd, tipologia, strlen(tipologia)) < 0) {
			perror("write");
			return EXIT_FAILURE;
		}

		if (write_all(sd, località, strlen(località)) < 0) {
			perror("write");
			return EXIT_FAILURE;
		}

		/* Leggo risposta */
		for (;;) {
			char response_line[1024];
			size_t response_line_len;

			// &array[0] == array
			memset(response_line, 0, sizeof(response_line));
			response_line_len = sizeof(response_line) - 1;

			err = rxb_readline(&rxb, sd, response_line, &response_line_len);
			if (err < 0) {
				perror("rxb_readline");
				return EXIT_FAILURE;
			}

#ifdef USE_LIBUNISTRING
			if (u8_check((uint8_t *)response_line, response_line_len) != NULL) {
				fprintf(stderr, "Stringa di risposta non testo UTF-8 valido\n");
				return EXIT_FAILURE;
			}
#endif

			if (strcmp(response_line, "---END RESPONSE---") == 0) {
				break;
			}

			puts(response_line);
		}

	}

	/* Deallocazione buffer interno readline */
	rxb_destroy(&rxb);

	/* Chiusura socket */
	close(sd);

	return EXIT_SUCCESS;
}

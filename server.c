/* Autores:
        Pietro Polinari Cavassin GRR20190430
        Richard Fernando Heise Ferreira GRR20191053
*/

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define TAMFILA        5
#define ERROR_CLINE   -1
#define ERROR_GETIP   -2
#define ERROR_OPENSCK -3
#define ERROR_NOBIND  -4
#define ERROR_ALLOC   -5
#define MAXHOSTNAME   30

/*
	Função que dobra o tamanho do vetor (vec) e seu tamanho (sz),
	retornando ponteiro para o novo vetor.
*/
unsigned int *double_array_size(unsigned int *vec, size_t *sz){
	unsigned int *auxptr;

	/* tenta realocar vetor e colocar no ponteiro auxiliar */
	auxptr = realloc(vec, (*sz)*2 * sizeof(unsigned int));
	if (!auxptr){
		puts ("Couldn't reallocate array of vec messages");
		free(vec);
		exit (ERROR_ALLOC);
	}

	/* seta novos bits para 0 e dobra tamanho */
	memset(auxptr+(*sz), 0, (*sz)*sizeof(unsigned int)); 
	(*sz) *= 2;
	return auxptr;
}

int main ( int argc, char *argv[] ) {

	int send_socket;
	unsigned int i;
    unsigned int *received;
    char buf[BUFSIZ + 1];
	struct sockaddr_in sa, isa;  /* sa: server, isa: client */
	struct hostent *hp;
	char localhost[MAXHOSTNAME];
	size_t sz = 1024;

    if (argc != 2) {
        puts("Correct use: server <port>");
        exit(ERROR_CLINE);
    }

	/* Coloca na string localhost o nome do host atual */
	gethostname(localhost, MAXHOSTNAME);

	/* Procura a struct hostent correspondente ao localhost */
	if ( (hp = gethostbyname(localhost)) == NULL) {
		puts ("Couldn't get my IP. Aborting");
		exit (ERROR_GETIP);
	}	
	
	/* Configura porta, endereço e tipo de endereço do host no socket. */
	sa.sin_port = htons(atoi(argv[1]));
	bcopy ((char *) hp->h_addr, (char *) &sa.sin_addr, hp->h_length);
	sa.sin_family = hp->h_addrtype;		


	/* Cria socket */
	if ((send_socket= socket(hp->h_addrtype,SOCK_DGRAM,0)) < 0){
        puts("Couldn't open send_socket. Aborting;" );
		exit(ERROR_OPENSCK);
	}	

	/* Especifica a porta do serviço */
	if (bind(send_socket, (struct sockaddr *) &sa,sizeof(sa)) < 0){
		puts ("Couldn't bind. Aborting.");
		exit (ERROR_NOBIND);
	}		

	/* Aloca vetor de mensagens recebidas e inicializa valores para 0 */
	received = malloc(sz * sizeof(unsigned int));
	if (!received){
		puts ("Couldn't allocate array of received messages");
		exit (ERROR_ALLOC);
	}
	memset(received, 0, sz*sizeof(unsigned int));

	/* Configura timeout do socket */
	struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    setsockopt(send_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

	/* Inicializa buffer de mensagens recebidas para 0 */
    memset(buf, 0, BUFSIZ);

	/* Inicializa timestamp para 0. Será útil para detecção de mensagens fora de ordem. */
	int timestamp = 1;
	fprintf(stderr, "Accepting messages...\n");
    while (1) {
        unsigned int szisa = sizeof(isa);  

		/* Aguarda e recebe mensagem do cliente. Se der timeout, sai do loop. */
        recvfrom(send_socket, buf, BUFSIZ, 0, (struct sockaddr *) &isa, &szisa);
		if (errno == EWOULDBLOCK){
			fprintf(stderr, "Server Timeout.\n");
			break;
		}

		/* Registra mensagem recebida. Se vetor de recebidas não for grande o suficiente, dobra o tamanho. */
		unsigned int num_message = atoi(buf);
		while(num_message >= sz)
			received = double_array_size(received, &sz);
		received[num_message] = timestamp;
	 	
		timestamp++;
	}

	/* Registra cabeçalho do log de mensagens perdidas */
	#ifndef CSV_FORMAT
	fprintf(stderr, "========== FINAL REPORT ==========\n");
	fprintf(stderr, "+--------------------------------+\n");
	fprintf(stderr, "|        Missing Messages        |\n");
	fprintf(stderr, "+--------------------------------+\n");
	#endif


	/* Registra no log todos os intervalos de mensagens não recebidas */
	unsigned int begin;                 // Início de vacância de mensagens
	i = 1;
	while (i < sz){
        
        /* Reporta no log intervalo atual de mensagens perdidas */
		if (!received[i]){
			begin = i;
			while (i < sz && !received[i])
				i++;
            
            /* Mensagens recebidas terminaram. */
			if (i == sz) break;
            
			#ifndef CSV_FORMAT
			fprintf(stderr, " Interval [%d .. %d] missing\n", begin, i-1);
			#endif
		}
		i++;
	}
	

	/* Registra cabeçalho do log de mensagens fora de ordem. */
	#ifndef CSV_FORMAT
	fprintf(stderr, "----------------------------------\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "+--------------------------------+\n");
	fprintf(stderr, "|      Out of Order Messages     |\n");
	fprintf(stderr, "+--------------------------------+\n");
	#endif

	
    /* 
        Registra no log todas as mensagens fora de ordem. 
        Também conta mensagens recebidas e maior mensagem.
    */
	i = 0;
	unsigned int out_of_order_cnt = 0;  // Contador de mensagens fora de ordem
	unsigned int total_received = 0;    // Total de mensagens recebidas
	unsigned int highest = 0;           // Maior mensagem recebida
	unsigned int highest_ts = 0;           // Timestamp da mensagem anterior
	unsigned int highest_msg = 0;          // Numero da mensagem anterior
	while (i < sz){
        
		if (received[i]){
            /* Caso haja timestamps decrescentes, reporta fora de ordem */
			if (received[i] < highest_ts){
                #ifndef CSV_FORMAT
                fprintf(stderr, " %d came before %d\n", i, highest_msg);
                #endif
                out_of_order_cnt++;
            }

            /* Registra maior mensagem, total de mensagens, maior timestamp e sua mensagem, */
            if (received[i] > highest_ts){
                highest_ts = received[i];
                highest_msg = i;
            }
			highest = i;
			total_received++;
		}
		i++;
	}
	
    /* Conta total de mensagens. */
	unsigned int lost = highest - total_received;


    /* Registra no log métricas finais das mensagens. */
	#ifndef CSV_FORMAT
	fprintf(stderr, "----------------------------------\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Highest message:\t %6d\n", highest);
	fprintf(stderr, "Messages received:\t %6d (%.2f%%)\n", total_received, (float)(total_received*100)/highest);
	fprintf(stderr, "Messages lost:\t\t %6d (%.2f%%)\n", lost , (float)(lost*100)/highest);
	fprintf(stderr, "Out of order messages:\t %6d (%.2f%%)\n", out_of_order_cnt, (float)(out_of_order_cnt*100)/highest);
	#endif


    /* Se foi solicitado resultado em formato CSV, imprime linha da execução atual */
	#ifdef CSV_FORMAT
	printf("%d,%d,%d,%d\n", highest, total_received, lost, out_of_order_cnt);
	#endif

	free(received);
    close(send_socket);
	return 0;
}

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

unsigned int *double_array_size(unsigned int *received, size_t *sz){
	unsigned int *auxptr;

	// tenta realocar vetor e colocar no vetor auxiliar
	auxptr = realloc(received, (*sz)*2 * sizeof(unsigned int));
	if (!auxptr){
		puts ("Couldn't reallocate array of received messages");
		free(received);
		exit (ERROR_ALLOC);
	}

	// seta novos bits para 0
	memset(auxptr+(*sz), 0, (*sz)*sizeof(unsigned int)); 
	(*sz) *= 2;
	return auxptr;
}

int main ( int argc, char *argv[] ) {

	int send_socket;
	unsigned int i;
    char buf[BUFSIZ + 1];
	struct sockaddr_in sa, isa;  /* sa: server, isa: client */
	struct hostent *hp;
	char localhost[MAXHOSTNAME];
	size_t sz = 1024;

    if (argc != 2) {
        puts("Correct use: server <port>");
        exit(ERROR_CLINE);
    }

	gethostname(localhost, MAXHOSTNAME);

	if ( (hp = gethostbyname(localhost)) == NULL) {
		puts ("Couldn't get my IP. Aborting");
		exit (ERROR_GETIP);
	}	
	
	sa.sin_port = htons(atoi(argv[1]));

	bcopy ((char *) hp->h_addr, (char *) &sa.sin_addr, hp->h_length);

	sa.sin_family = hp->h_addrtype;		


	if ((send_socket= socket(hp->h_addrtype,SOCK_DGRAM,0)) < 0){
        puts("Couldn't open send_socket. Aborting;" );
		exit(ERROR_OPENSCK);
	}	

	if (bind(send_socket, (struct sockaddr *) &sa,sizeof(sa)) < 0){
		puts ("Couldn't bind. Aborting.");
		exit (ERROR_NOBIND);
	}		


	unsigned int *received = malloc(sz * sizeof(unsigned int));
	if (!received){
		puts ("Couldn't allocate array of received messages");
		exit (ERROR_ALLOC);
	}
	memset(received, 0, sz*sizeof(unsigned int));

	struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    setsockopt(send_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);


    memset(buf, 0, BUFSIZ);

	int timestamp = 1;
	printf("Accepting messages...\n");
    while (1) {
        unsigned int szisa = sizeof(isa); 

        recvfrom(send_socket, buf, BUFSIZ, 0, (struct sockaddr *) &isa, &szisa);
		if (errno == EWOULDBLOCK){
			printf("Server Timeout.\n");
			break;
		}

		// registra mensagem recebida
		unsigned int num_message = atoi(buf);
		while(num_message >= sz)
			received = double_array_size(received, &sz);
		received[num_message] = timestamp;
	 	// printf("Timestamp: %d\n", timestamp);
		timestamp++;
	}

	printf("========== FINAL REPORT ==========\n");
	printf("+--------------------------------+\n");
	printf("|        Missing Messages        |\n");
	printf("+--------------------------------+\n");


	unsigned int total_received = 0;
	unsigned int begin;
	unsigned int highest = 0;
	unsigned int out_of_order_cnt = 0;
	i = 1;
	while (i < sz){
		if (!received[i]){
			begin = i;
			while (i < sz && !received[i])
				i++;
			if (i == sz) break;
			printf(" Interval [%d .. %d] missing\n", begin, i-1);
		} else {
			highest = i;
			total_received++;
		}
		i++;
	}
	
	printf("----------------------------------\n");
	printf("\n");
	printf("+--------------------------------+\n");
	printf("|      Out of Order Messages     |\n");
	printf("+--------------------------------+\n");

	i = 0;
	unsigned int last_ts = 0;
	unsigned int last_msg = 0;
	while (i < sz){
		if (received[i]){
			if (received[i] < last_ts)
				printf(" %d came before %d\n", i, last_msg);

			last_ts = received[i];
			last_msg = i;
		}
		i++;
	}
	
	printf("----------------------------------\n");
	printf("\n");
	

	unsigned int lost = highest - total_received;
	printf("Highest message:\t %6d\n", highest);
	printf("Messages received:\t %6d (%.2f%%)\n", total_received, (float)(total_received*100)/highest);
	printf("Messages lost:\t\t %6d (%.2f%%)\n", lost , (float)(lost*100)/highest);
	printf("Out of order messages:\t %6d (%.2f%%)\n", out_of_order_cnt, (float)(out_of_order_cnt*100)/highest);

	free(received);
	return 0;
}

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

#define TAMFILA        5
#define ERROR_CLINE   -1
#define ERROR_GETIP   -2
#define ERROR_OPENSCK -3
#define ERROR_NOBIND  -4
#define ERROR_ALLOC   -5
#define MAXHOSTNAME   30

char *double_array_size(char *received, size_t *sz){
	char *auxptr;

	// tenta realocar vetor e colocar no vetor auxiliar
	auxptr = realloc(received, (*sz)*2 * sizeof(char));
	if (!auxptr){
		puts ("Couldn't reallocate array of received messages");
		free(received);
		exit (ERROR_ALLOC);
	}

	// seta novos bits para 0
	memset(auxptr+(*sz), 0, (*sz)*sizeof(char)); 
	(*sz) *= 2;
	return auxptr;
}

int main ( int argc, char *argv[] ) {

	int send_socket, recv_socket;
	unsigned int i;
    char buf[BUFSIZ + 1];
	struct sockaddr_in sa, isa;  /* sa: server, isa: client */
	struct hostent *hp;
	char localhost[MAXHOSTNAME];
	size_t sz = 2;

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

	char *received = malloc(sz * sizeof(char));
	if (!received){
		puts ("Couldn't allocate array of received messages");
		exit (ERROR_ALLOC);
	}

    while (1) {
        memset(buf, 0, BUFSIZ);
        int szisa = sizeof(isa); 

        puts("Awaiting message.");
        recvfrom(send_socket, buf, BUFSIZ, 0, (struct sockaddr *) &isa, &szisa);
        printf("I'm the server, just received a message ----> %s\n", buf);

		// registra mensagem recebida
		unsigned int num_message = atoi(buf);
		while(num_message >= sz)
			received = double_array_size(received, &sz);
		received[num_message] = 1;

        sendto(send_socket, buf, BUFSIZ, 0, (struct sockaddr *) &isa, szisa);
	}

	free(received);
	return 0;
}

/* Autores:
        Pietro Polinari Cavassin GRR20190430
        Richard Fernando Heise Ferreira GRR20191053
*/

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
#define MAXHOSTNAME   30

main ( int argc, char *argv[] ) {

	int send_socket, recv_socket;
	unsigned int i;
    char buf[BUFSIZ + 1];
	struct sockaddr_in sa, isa;  /* sa: server, isa: client */
	struct hostent *hp;
	char localhost[MAXHOSTNAME];

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
 
    while (1) {
        memset(buf, 0, BUFSIZ);
        int szisa = sizeof(isa); 
        puts("Awaiting message.");
        recvfrom(send_socket, buf, BUFSIZ, 0, (struct sockaddr *) &isa, &szisa);
        printf("I'm the server, just received a message ----> %s\n", buf);
        sendto(send_socket, buf, BUFSIZ, 0, (struct sockaddr *) &isa, szisa);
	}
}

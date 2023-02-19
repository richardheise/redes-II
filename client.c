/* Autores:
        Pietro Polinari Cavassin GRR20190430
        Richard Fernando Heise Ferreira GRR20191053
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>

#define ERROR_CLINE -1
#define ERROR_GETIP -2
#define ERROR_OPENSCKT -3
#define ERROR_SENDATA -4

int main(int argc, char *argv[]) { 

    int socket_description;
    // int number_recv_bytes;
    struct sockaddr_in sa;
    struct hostent *hp;
    char buf[BUFSIZ+1];
    char *host;

    if(argc != 4) {
      puts("Correct usage: <client> <server-name> <port> <data>");
      exit(ERROR_CLINE);
    }

    host = argv[1];
    int num_message = atoi(argv[3]);

    if( (hp = gethostbyname(host)) == NULL ) {
      puts("Nao consegui obter endereco IP do servidor.");
      exit(ERROR_GETIP);
    }

    bcopy( (char *)hp->h_addr, (char *)&sa.sin_addr, hp->h_length );
    sa.sin_family = hp->h_addrtype;

    sa.sin_port = htons(atoi(argv[2]));

    if( (socket_description=socket(hp->h_addrtype, SOCK_DGRAM, 0)) < 0 ) {
      puts("Couldn't open socket.");
      exit(ERROR_OPENSCKT);
    }

    char data[10] = {0};

    for (int i = 1; i <= num_message; i++) {
        memset(buf, 0, BUFSIZ+1);

        sprintf(data, "%d ", i);

        if( sendto(socket_description, data, strlen(data)+1, 0, (struct
            sockaddr *) &sa, sizeof sa) != strlen(data)+1) {
            puts("Couldn't send data."); 
            exit(ERROR_SENDATA);
        }

        // int szsa = sizeof(sa);
        // int recvres = recvfrom(socket_description, buf, BUFSIZ, 0, (struct sockaddr *) &sa, &szsa);
        // printf("I'm the client, just received %d, data: %s\n", recvres, buf);
    }

   
    close(socket_description);
    return 0;
}

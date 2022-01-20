
/*
    TFTP Client : Receive a file from the server
    Usage: gettftp host file
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>  // (void *malloc(size_t size))
#include <unistd.h>
#include <string.h>

#include <netdb.h>
#include <arpa/inet.h>  // (uint16_t htons(uint16_t hostshort);)
#include <netinet/in.h>

#include <arpa/tftp.h>  // (struct tftphdr defined in it)

char buffer[512];


// ==== Functions (definition) ====

void checkEnoughArgs(int argc, char **argv, char **host, char **file);

void initHints(struct addrinfo *hints);
void getServAddr(char *host, struct addrinfo *hints, struct addrinfo *result);

int createSocket(struct addrinfo *result);

void sendRRQ(int sfd, struct addrinfo *result, char *fileName, char *mode);
void fillPacket(char **packet, char *fileName, char *mode);


// ==== MAIN ====

int main (int argc, char **argv) {

    // ---- Get the arguments from Terminal
    char *host, *file;
    checkEnoughArgs(argc, argv, &host, &file);
    // printf("%s, %s", host, file);

    // ---- Get the server address
    struct addrinfo hints, *result = NULL;
    initHints(&hints);
    // printf("%d %d %d", hints.ai_family, hints.ai_socktype, hints.ai_protocol);
    getServAddr(host, &hints, result);

    // ---- Create a socket
    int sockfd = createSocket(result);
    // printf("%d", sockfd);

    // ---- Form and send a Read Request (RRQ)
    sendRRQ(sockfd, result, file, "octet");
    
}


// ==== Functions (implementation) ====

void checkEnoughArgs(int argc, char **argv, char **host, char **file) {

    if (argc < 3) { // Not enough arguments
        fprintf(stderr, "Usage: %s host filename\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    *host = argv[1];
    *file = argv[2];

}

void initHints(struct addrinfo *hints) {
    memset(hints, 0, sizeof(struct addrinfo));

    hints->ai_family = AF_INET;
    hints->ai_socktype = SOCK_DGRAM;
    hints->ai_protocol = IPPROTO_UDP;
}

void getServAddr(char *host, struct addrinfo *hints, struct addrinfo *result) {
    
    char *port = "69";  // 1069 (local) else 69 (external)

    int s = getaddrinfo(host, port, hints, &result);

    if (s != 0) {   // error getting the address info?
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

}

int createSocket(struct addrinfo *result) {
    int sfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

    if (sfd < 0) {
		perror("Socket Failure");
		exit(EXIT_FAILURE);
	}

    freeaddrinfo(result);

    return sfd;

}

void sendRRQ(int sfd, struct addrinfo *result, char *fileName, char *mode) {
    char *packet;
    int packetSize = 2 + strlen(fileName)+1+strlen(mode)+1;
    packet = (char *) malloc(packetSize);

    fillPacket(&packet, fileName, mode);

    sendto(sfd, packet, packetSize, 0, (struct sockaddr *) result->ai_addr, result->ai_addrlen);

    packet = (char *) realloc(packet, packetSize);

}

void fillPacket(char **packet, char *fileName, char *mode) {

    (*packet)[0] = 0;
    (*packet)[1] = 1;

    char *cp = &((*packet)[2]);

    strcpy(cp, fileName);
    cp += strlen(fileName);

    *cp++ = '\0';

    strcpy(cp, mode);
    cp += strlen(mode);

    *cp++ = '\0';

}
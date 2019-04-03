#include <stdio.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>


int main(void) {
    /* Initialisation du serveur */
    int dS = socket(PF_INET, SOCK_STREAM, 0);
    struct sockaddr_in ad;
    ad.sin_family = AF_INET;
    ad.sin_addr.s_addr = INADDR_ANY;
    ad.sin_port = 0; 
    socklen_t len = sizeof(ad);

    /* Assignation d'un port aléatoire */
    if (getsockname(dS, (struct sockaddr*) &ad, &len) < 0) {
        printf("Erreur lors de l'initialisation");
        exit(1);
    } else {
        printf("Adresse IP : %s\n", inet_ntoa(ad.sin_addr));
        printf("Port : %d\n", ntohs(ad.sin_port));
    }

    return 0;
     

}
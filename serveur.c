#include <stdio.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>


int quit = 0;

void traitement(int n) {
    quit = 1;
}

int main(void) {
    /* Déclaration des variables */
    int dS, res, dSC, dSC2, stop = 1;

    char msg[280];


    /* Initialisation du serveur */
    dS = socket(PF_INET, SOCK_STREAM, 0);
    struct sockaddr_in ad;
    ad.sin_family = AF_INET;
    ad.sin_addr.s_addr = INADDR_ANY;  
    res = bind(dS, (struct sockaddr*) &ad, sizeof(ad));
    /* */
    printf("%d\n", res);
    printf("Port : %d\n", ntohs(ad.sin_port));

    /**/  
    listen(dS, 10);
    struct sockaddr_in aC;
    socklen_t lg = sizeof(struct sockaddr_in);
    /* Tant que control+C n'est pas préssé pour arrêter le serveur */
    while (quit != 1) {
        /* Mise en place de la communication avec les 2 clients */
        dSC = accept(dS, (struct sockaddr*) &aC, &lg);
        dSC2 = accept(dS, (struct sockaddr*) &aC, &lg);
        stop = 1;
        /* Tant que la connexion n'a pas été perdue avec un des 2 clients */
        while (stop > 0) {
            /* Réception du message du client émetteur */
            stop = recv(dSC, msg, strlen(msg), 0);
            stop = send(dSC2, msg, strlen(msg), 0);
            /* Réception du message du 2e client */
            stop = recv(dSC2, msg, strlen(msg), 0);
            stop = send(dSC, msg, strlen(msg), 0);
            
        }
        stop = 1;
        /* Fermeture de la communication avec les 2 clients */
        close(dSC);
        close(dSC2);
        /* Check si un control+C a été lancé */
        signal(SIGINT, traitement);
    }
    /* Arrêt du serveur et fin */
    close(dS);
    return 0;
     

}
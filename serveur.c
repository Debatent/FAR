#include <stdio.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

void vidermemoiretamponclavier(void){
    /*vide la mémoire tampon du clavier*/
    char c;
    while( (c=getchar())!='\n' && c!=EOF);
}


int main(void) {
    /* Déclaration des variables */
    int dS, res, dSC, dSC2, deb = 0;
    char msg[280];

    /* Initialisation du serveur */
    dS = socket(PF_INET, SOCK_STREAM, 0);
    struct sockaddr_in ad;
    ad.sin_family = AF_INET;
    ad.sin_addr.s_addr = INADDR_ANY;
    ad.sin_port = 45001;
    res = bind(dS, (struct sockaddr*) &ad, sizeof(ad));

    /* Affichage du port */
    if (res == 0) {
	    printf("Initialisation du serveur terminée !\n");
    } else {
	    return -1;
    }
    printf("Port : %d\n", ad.sin_port);

    /* Attente des connections */
    listen(dS, 10);
    struct sockaddr_in aC;
    socklen_t lg = sizeof(struct sockaddr_in);

    /* Boucle principale pour les connexions des clients */
    while (1) {
	    /* Condition d'arrêt du serveur */
	    if (deb == 1) {
	        printf("Voulez vous arrêter le serveur ? o/n\n");
	        if (getchar() == 'o') {
	        break;
	        }
	    vidermemoiretamponclavier();
        }
	    deb = 1;
	    printf("En attente de connection ...\n");
        /* Mise en place de la communication avec les 2 clients */
        dSC = accept(dS, (struct sockaddr*) &aC, &lg);
        dSC2 = accept(dS, (struct sockaddr*) &aC, &lg);

        /* Boucle d'échange des messages entre les 2 clients */
        while (1) {
	        bzero(msg, 280);

            /* Réception du message du client émetteur */
            recv(dSC, msg, sizeof(msg), 0);
	        printf("Reçu : %s\n", msg);

	        /* Envoi du message au second client */
	        send(dSC2, msg, sizeof(msg), 0);

	        /* Condition d'arrêt de la communication (message = fin) */
	        if (strcmp("fin", msg) == 0) {
		        printf("Fin de la communication\n");
		        break;
	        }
	        printf("Message envoyé au second client...\n");
	        bzero(msg, 280);

            /* Réception de la réponse du second client */
            recv(dSC2, msg, sizeof(msg), 0);
	        printf("Reçu : %s\n", msg);

	        /* Envoi de la réponse au premier client */
	        send(dSC, msg, sizeof(msg), 0);

	        /* Condition d'arrêt de la communication (message = fin) */
	        if (strcmp("fin", msg) == 0) {
		        printf("Fin de la communication\n");
		        break;
	        }
	        printf("Message envoyé au premier client...\n");
        }

    }

    /*Arrêt du serveur et fin */
    printf("Arrêt du serveur...\n");
    close(dS);
    return 0;

}

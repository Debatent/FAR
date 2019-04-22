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
#include <pthread.h>

#define TAILLEMAX 25

int tabSockets[TAILLEMAX] = { 0 };


void vidermemoiretamponclavier(void)
{
    /*vide la mémoire tampon du clavier*/
    char c;
    while ((c = getchar()) != '\n' && c != EOF);
}

/* Structure des arguments passés au thread (client 1 et client 2)*/
struct thread_args {
    int dSC;
    char pseudo[280];
};

/* Thread qui réceptionne le message d'un client et l'envoie à l'autre en boucle  */
void *threadEnvoi(void *args) {
    char msg[280];
    char messageComplet[280];
    int res;
    char pseudo[280];
    int dSC;
    /* Récupération des arguments */
    struct thread_args *arguments = (struct thread_args *) args;
    /* Réception du pseudo */
    dSC = arguments->dSC;
    recv(dSC, pseudo, sizeof(pseudo), 0);
    printf("Pseudo : %s\n", pseudo);

    /* Condition d'arrêt : Pas de réception (le client a mis fin à la connexion) */
    while (1) {
        /* En attente de réception d'un message */
        printf("ENTREE DANS LE THREAD AVEC DSC : %d\n", dSC);
        res = recv(dSC, msg, sizeof(msg),0);
        if (res == 0) {
            break;
        }
        /* Si le message reçu est "fin" */
        if (strcmp(msg,"fin") == 0){
            /* Formattage du message à envoyer */
            strcpy(messageComplet, pseudo);
            strcat(messageComplet, " a quitté la discussion\0");
            /* Envoi à tous les clients */
            for (int i =0; i < TAILLEMAX-1; i++) {
                if (tabSockets[i] != 0) {
                    /* Suppression du socket du client ayant émit "fin" dans le tableau */
                    if (tabSockets[i] == dSC) {
                        tabSockets[i] = 0;
                    } else {
                        printf("ENVOI NUMERO FIN %d\n", i);
                        res = send(tabSockets[i], messageComplet, sizeof(messageComplet), 0);
                        if (res == 0) {
                            tabSockets[i] = 0;
                        }
                    }
                }
            }
            break;
        }          
        /* Formattage du message à envoyer */ 
        strcpy(messageComplet, pseudo);
        strcat(messageComplet, " : ");
        strcat(messageComplet, msg);
        printf("%s\n", messageComplet);
        /* Envoi à tous les clients */
        for (int i = 0; i < TAILLEMAX-1; i++) {
            if (tabSockets[i] != 0 && tabSockets[i] != dSC) {
                printf("ENVOI AU CLIENT %d\n", tabSockets[i]);
                res = send(tabSockets[i], messageComplet, sizeof(messageComplet), 0);
                if (res == 0) {
                    tabSockets[i] = 0;
                }
            }
        }
        bzero(msg, 280);
        bzero(messageComplet, 280);

    }
    return NULL;
}

int main(void)
{
    /* Déclaration des variables */
    int dS, res, i=0;
    pthread_t tid[TAILLEMAX];

    /* Déclaration des structures */
    struct thread_args arguments;

    /* Initialisation du serveur */
    dS = socket(PF_INET, SOCK_STREAM, 0);
    struct sockaddr_in ad;
    ad.sin_family = AF_INET;
    ad.sin_addr.s_addr = INADDR_ANY;
    ad.sin_port = 45001;
    res = bind(dS, (struct sockaddr *)&ad, sizeof(ad));

    /* Affichage du port */
    if (res == 0)
    {
        printf("Initialisation du serveur terminée !\n");
        printf("Port : %d\n", ad.sin_port);
    }
    else
    {
        printf("Erreur dans l'initialisation du serveur\n");
        return -1;
    }

    /* Attente des connexions */
    listen(dS, 10);
    struct sockaddr_in aC;
    socklen_t lg = sizeof(struct sockaddr_in);

    /* Boucle principale pour les connexions des clients */
    while (1)
    {
        /* Condition d'arrêt du serveur, seulement si le tableau est rempli de 0 et qu'on a déja itéré une fois */
        if (deb == 1)
        {
            printf("Voulez vous arrêter le serveur ? o/n\n");
            if (getchar() == 'o')
            {
                break;
            }
            vidermemoiretamponclavier();
            i = 0;
        }
        deb = 1;

        /* Attente de connexions entrantes */
        arguments.dSC = accept(dS, (struct sockaddr *)&aC, &lg);
        printf("Client DSC : %d\n", arguments.dSC);

        /* Ajout du socket du client au tableau de sockets */
        tabSockets[i] = arguments.dSC;
        printf("Tableau : %d\n", tabSockets[i]);
        /* Création du thread permettant la transmission des messages */
        pthread_create(&tid[i], NULL, threadEnvoi, (void *)&arguments);
        i++;

    }

    /*Arrêt du serveur et fin */
    printf("Arrêt du serveur...\n");
    close(dS);
    return 0;
}

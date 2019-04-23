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

int dS;

/* Traitement du signal CTRL+C (condition d'arrêt du serveur) */
void traitement (int n){
    close(dS);
    puts("Arret serveur");
    _exit(0);
}

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
    struct sockaddr_in aC;
};

/* Déclaration du tableau de sockets et remplissage avec des 0 */
struct thread_args tabSockets[TAILLEMAX] = { 0 };

/* Renvoi le nombre de clients connectés à la discussion */
int getNbConnectes() {
    int count = 0;
    for (int i = 0; i < TAILLEMAX-1; i++) {
        if (tabSockets[i].dSC != 0) {
            count++;
        }
    }
    return count;
}

int getPseudo(char * pseudo) {
    printf("%s\n", pseudo);
    for (int i = 0; i < TAILLEMAX-1; i++) {
        if (strcmp(tabSockets[i].pseudo, pseudo) == 0) {
            return 0;
        }
    }
    return -1;
}

/* Thread qui réceptionne le message d'un client et l'envoie à l'autre en boucle  */
void *threadEnvoi(void * numCli) {
    char msg[280];
    char messageComplet[280];
    int res, fin = 0;
    char pseudo[280];
    int num = *((int *) numCli);
    printf("%d\n", num);
    int dSC = tabSockets[num].dSC;


    printf("Pseudo : %s\n", pseudo);
    printf("ID du client : %d\n", dSC);
    printf("Adresse IP : %s\n", inet_ntoa(tabSockets[num].aC.sin_addr));
    printf("Port : %d\n",(int) ntohs(tabSockets[num].aC.sin_port));

    /* Condition d'arrêt : Pas de réception (le client a mis fin à la connexion) */
    while (1) {
        /* En attente de réception d'un message */
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
                if (tabSockets[i].dSC != 0) {
                    /* Suppression du socket du client ayant émit "fin" dans le tableau */
                    if (tabSockets[i].dSC == dSC) {
                        tabSockets[i].dSC = 0;
                    } else {
                        printf("ENVOI AU CLIENT %d\n", i);
                        res = send(tabSockets[i].dSC, messageComplet, sizeof(messageComplet), 0);
                        if (res == 0) {
                            tabSockets[i].dSC = 0;
                        }
                    }
                }
            }
            break;
        /* Si le message reçu est "file" */
        } else if (strcmp(msg, "file") == 0) {
            /* Récupération du pseudo du client auquel le client veut envoyer le fichier */
            

        }
        /* Formattage du message à envoyer */
        strcpy(messageComplet, pseudo);
        strcat(messageComplet, " : ");
        strcat(messageComplet, msg);
        printf("%s\n", messageComplet);
        /* Envoi à tous les clients */
        for (int i = 0; i < TAILLEMAX-1; i++) {
            if (tabSockets[i].dSC != 0 && tabSockets[i].dSC != dSC) {
                printf("ENVOI AU CLIENT %d\n", tabSockets[i].dSC);
                res = send(tabSockets[i].dSC, messageComplet, sizeof(messageComplet), 0);
                if (res == 0) {
                    tabSockets[i].dSC = 0;
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
    int res, i=0;
    pthread_t tid[TAILLEMAX];

    /* Initialisation du serveur */
    dS = socket(PF_INET, SOCK_STREAM, 0);
    struct sockaddr_in ad;
    ad.sin_family = AF_INET;
    ad.sin_addr.s_addr = INADDR_ANY;
    ad.sin_port = 45000;
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
    char msg[280];
    int *arg;

    signal(SIGINT,traitement);
    /* Boucle principale pour les connexions des clients */
    while (1)
    {
        /* Attente de connexions entrantes */
        tabSockets[i].dSC = accept(dS, (struct sockaddr *)&aC, &lg);
        tabSockets[i].aC = aC;
        printf("Client DSC : %d\n", tabSockets[i].dSC);
        sprintf(msg, "Il y a actuellement %d connecté(s)", getNbConnectes());
        send(tabSockets[i].dSC, msg, sizeof(msg), 0);
        /* Création du thread permettant la transmission des messages */
        *arg = i;
        pthread_create(&tid[i], NULL, threadEnvoi, arg);
        i++;

    }

    /*Arrêt du serveur et fin */
    printf("Arrêt du serveur...\n");
    close(dS);
    return 0;
}

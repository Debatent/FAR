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
// Check si le pseudo existe
int getPseudo(char * pseudo) {
    for (int i = 0; i < TAILLEMAX-1; i++) {
        if (strcmp(tabSockets[i].pseudo, pseudo) == 0) {
            return 0;
        }
    }
    return -1;
}
//Check si le pseudo existe et renvoie le numéro de socket correspondant
int getIdByPseudo(char * pseudo) {
    for (int i = 0; i < TAILLEMAX-1; i++) {
        if (strcmp(tabSockets[i].pseudo, pseudo) == 0) {
            return tabSockets[i].dSC;
        }
    }
    return -1;
}

//Récupère la position de la socket en entré
int getPosBySocket(int sock) {
    printf("LA SOCKET EN PARAM %d\n", sock);
    for (int i = 0; i < TAILLEMAX-1; i++) {
        printf("SOKCET NUM %d\n", tabSockets[i].dSC);
        if (tabSockets[i].dSC == sock) {
            return i;
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
    int i = (int) numCli;
    printf("%d\n", i);
    int dSC = tabSockets[i].dSC;
    /* Réception du pseudo */
    while (fin != 1) {
        recv(dSC, pseudo, sizeof(pseudo), 0);
        /* Si le pseudo est déjà pris, on demande d'en rentrer un autre */
        if (getPseudo(pseudo) == 0) {
            strcpy(msg, "-1");
            printf("EXISTANT : %s\n", msg);
            send(dSC, msg, sizeof(msg), 0);
        } else {
            strcpy(msg, "0");
            printf("OK : %s\n", msg);
            send(tabSockets[i].dSC, msg, sizeof(msg), 0);
            strcpy(tabSockets[i].pseudo, pseudo);
            fin = 1;
        }
        bzero(msg, 280);
    }

    printf("Pseudo : %s\n", pseudo);
    printf("ID du client : %d\n", dSC);
    printf("Adresse IP : %s\n", inet_ntoa(tabSockets[i].aC.sin_addr));
    printf("Port : %d\n",(int) ntohs(tabSockets[i].aC.sin_port));

    /* Condition d'arrêt : Pas de réception (le client a mis fin à la connexion) */
    while (1) {
        /* En attente de réception d'un message */
        res = recv(dSC, msg, sizeof(msg),0);
        if (res == 0) {
            break;
        }
        /* Si le message reçu est "fin" */
        if (strcmp(msg,"fin") == 0){
            printf("MESSAGE FIN RECU\n");
            /* Formattage du message à envoyer */
            strcpy(messageComplet, pseudo);
            strcat(messageComplet, " a quitté la discussion\0\n");
            /* Envoi à tous les clients */
            for (int i =0; i < TAILLEMAX-1; i++) {
                if (tabSockets[i].dSC != 0) {
                    /* Suppression du socket du client ayant émit "fin" dans le tableau */
                    if (tabSockets[i].dSC == dSC) {
                        tabSockets[i].dSC = 0;
                        strcpy(tabSockets[i].pseudo, "");
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
        } else if (strcmp(msg, "ile") == 0) {
            /* Récupération du pseudo du client auquel le client veut envoyer le fichier */
            bzero(msg, 280);
            recv(dSC, msg, sizeof(msg), 0);
            /* Check si le pseudo existe et envoie au second client l'adresse IP et le port */
            int sock = getIdByPseudo(msg);
            printf("RETOUR DE LA FONCTION : %d\n", sock);
            if (sock != -1) {
                strcpy(msg, pseudo);
                strcat(msg, " souhaite vous envoyer un fichier\n");
                send(sock, msg, sizeof(msg), 0);
                bzero(msg, 280);
                strcpy(msg, "ile");
                send(sock, msg, sizeof(msg), 0);
                bzero(msg, 280);
                //ENVOI AU CLIENT RECEPTEUR
                //Envoi du port 
                sprintf(msg, "%d", (int) ntohs(tabSockets[i].aC.sin_port));
                printf("PORT : %s\n", msg);
                send(sock, msg, sizeof(msg), 0);
                bzero(msg, 280);
                //Envoi de l'ip
                strcpy(msg, inet_ntoa(tabSockets[i].aC.sin_addr));
                printf("IP : %s\n", msg);
                send(sock, msg, sizeof(msg), 0);
                bzero(msg, 280);

                //ENVOI AU CLIENT EMETTEUR
                int pos = getPosBySocket(sock);
                if (pos != -1) {
                    //Envoi du port 
                    printf("PORT DE L'EMETTEUR i = %d\n", pos);
                    sprintf(msg, "%d", (int) ntohs(tabSockets[pos].aC.sin_port));
                    printf("PORT : %s\n", msg);
                    send(dSC, msg, sizeof(msg), 0);
                    bzero(msg, 280);
                    //Envoi de l'ip
                    strcpy(msg, inet_ntoa(tabSockets[pos].aC.sin_addr));
                    printf("IP : %s\n", msg);
                    send(dSC, msg, sizeof(msg), 0);
                }

            } else {
                strcpy(msg, "Ce pseudo n'est pas connecté, annulation du transfert\n");
                send(dSC, msg, sizeof(msg), 0);
            }
        } else {
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

    signal(SIGINT,traitement);
    /* Boucle principale pour les connexions des clients */
    while (1)
    {
        /* Attente de connexions entrantes */
        tabSockets[i].dSC = accept(dS, (struct sockaddr *)&aC, &lg);
        tabSockets[i].aC = aC;
        printf("Client DSC : %d\n", tabSockets[i].dSC);

        /* Création du thread permettant la transmission des messages */
        pthread_create(&tid[i], NULL, threadEnvoi, i);
        i++;

    }

    /*Arrêt du serveur et fin */
    printf("Arrêt du serveur...\n");
    close(dS);
    return 0;
}

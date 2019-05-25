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

#define TAILLEMAX 50

int dS, dSFiles;

/* Traitement du signal CTRL+C (condition d'arrêt du serveur) */
void traitement(int n)
{
    close(dS);
    puts("Arret serveur");
    _exit(0);
}

void vidermemoiretamponclavier(void)
{
    /*vide la mémoire tampon du clavier*/
    char c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

/* Structure des arguments passés au thread (client 1 et client 2)*/
struct thread_args
{
    int dSC;
    int dSC2;
    char nomChaine[30];
    char descrChaine[280];
    char pseudo[280];
    struct sockaddr_in aC;
    struct sockaddr_in aC2;
};

/* Déclaration du tableau de sockets et remplissage avec des 0 */
struct thread_args tabSockets[TAILLEMAX] = {0};

/* Renvoi le nombre de clients connectés à la discussion */
int getNbConnectes(char *salon)
{
    int count = 0;
    for (int i = 0; i < TAILLEMAX - 1; i++)
    {
        if (strcmp(tabSockets[i].nomChaine, salon) == 0) {
            if (tabSockets[i].dSC != 0) {
                count++;
            }
        } else {
            i += 10;
        }
    }
    return count;
}
// Check si le pseudo existe
int getPseudo(char *pseudo)
{
    for (int i = 0; i < TAILLEMAX - 1; i++)
    {
        if (strcmp(tabSockets[i].pseudo, pseudo) == 0)
        {
            return 0;
        }
    }
    return -1;
}
//Check si le pseudo existe et renvoie le numéro de socket correspondant
int getIdByPseudo(char *pseudo)
{
    for (int i = 0; i < TAILLEMAX - 1; i++)
    {
        if (strcmp(tabSockets[i].pseudo, pseudo) == 0)
        {
            puts("Pseudo reconnu");
            return tabSockets[i].dSC2;
        }
    }
    return -1;
}

//Récupère la position de la socket en entré
int getPosBySocket(int sock)
{
    printf("LA SOCKET EN PARAM %d\n", sock);
    for (int i = 0; i < TAILLEMAX - 1; i++)
    {
        if (tabSockets[i].dSC == sock)
        {
            return i;
        }
    }
    return -1;
}

char *getChaines() {
    char * chaines;
    chaines = malloc(sizeof(char)*2000);
    char nbCo[15];
    int i = 0;
    while (i <= TAILLEMAX)
    {
        if (strcmp(tabSockets[i].nomChaine, "") != 0)
        {
            strcat(chaines, tabSockets[i].nomChaine);
            strcat(chaines, " : ");
            strcat(chaines, tabSockets[i].descrChaine);
            strcat(chaines, "\nNombre de connectés : ");
            sprintf(nbCo, "%d\n", getNbConnectes(tabSockets[i].nomChaine));
            strcat(chaines, nbCo);
        }
        i += 10;
    }
    return chaines;

}
//retourne 0 si le salon existe, -1 sinon
int checkSalon(char* salon) {
    int i=0;
    while (i <= TAILLEMAX)
    {
        if (strcmp(tabSockets[i].nomChaine, salon) == 0)
        {
            return 0;
        } else {
            i += 10;
        }

    }
    return -1;
}

void *threadEnvoiFichier(void *numEmetteur)
{
    printf("DANS LE THREAD ENVOI FICHIER\n");
    int i = (int)numEmetteur;
    char msg[280];
    char pseudo[280], port[280];
    int dSC = tabSockets[i].dSC2;
    /* Récupération du pseudo du client auquel le client veut envoyer le fichier */
    bzero(msg, 280);
    printf("En attente du message de l'émetteur (pseudo) \n");
    recv(dSC, msg, sizeof(msg), 0);
    printf("PSEUDO : %s\n", msg);
    strcpy(pseudo, msg);
    // Réception du port
    recv(dSC, port, sizeof(port), 0);
    printf("Port : %s\n", port);
    /* Check si le pseudo existe et envoie au second client l'adresse IP et le port */
    int sock = getIdByPseudo(pseudo);
    printf("RETOUR DE LA FONCTION : %d\n", sock);
    if (sock != -1)
    {
        //ENVOI AU CLIENT RECEPTEUR
        //Envoi de l'ip
        strcpy(msg, inet_ntoa(tabSockets[i].aC2.sin_addr));
        printf("IP : %s\n", msg);
        send(sock, msg, sizeof(msg), 0);
        //Envoi du port
        printf("PORT : %s\n", port);
        send(sock, port, sizeof(port), 0);
        bzero(msg, 280);
    }
    else
    {
        strcpy(msg, "Ce pseudo n'est pas connecté, annulation du transfert\n");
        send(dSC, msg, sizeof(msg), 0);
    }
}

int inserer(char *nomSalon, int dSC, char* pseudo) {
    int i=0;
    while (i <= TAILLEMAX)
    {
        if (strcmp(tabSockets[i].nomChaine, nomSalon) == 0)
        {
            while (tabSockets[i].dSC != 0) {
                i += 1;
            }
            if (strcmp(tabSockets[i].nomChaine, nomSalon) == 0) {
                tabSockets[i].dSC = dSC;
                printf("Insertion du client à la position : %d\n", i);
                strcpy(tabSockets[i].pseudo, pseudo);
                return 0;
            } else {
                return 1;
            }
        } else {
            i += 10;
        }

    }
    return -1;
}

int salonExistant(char *nom) {
    int i=0;
    while (i <= TAILLEMAX)
    {
        if (strcmp(tabSockets[i].nomChaine, nom) == 0)
        {
            return -1;
        }
        i = i+1;
    }
    return 0;
}

int ajouterSalon(char *nom, char*description) {
    int i=0;
    while (i <= TAILLEMAX)
    {
        printf("Essai à la position : %d\n", i);
        if (strcmp(tabSockets[i].nomChaine, "") == 0)
        {
            for (int j = 0; j < 10; j++) {
                printf("Insertion à la position : %d\n", i+j);
                strcpy(tabSockets[i+j].nomChaine, nom);
                strcpy(tabSockets[i+j].descrChaine, description);
            }
            return 0;
        }
        i = i+10;   
    }
    return -1;
}

char *getSalonByPseudo(char * pseudo) {
    int i = 0;
    while (i <= TAILLEMAX)
    {
        if (strcmp(tabSockets[i].pseudo, pseudo) == 0)
        {
            return tabSockets[i].nomChaine;
        }
        i += 1;
    }
    return "erreur";

}

int editDescSalon(char *nomSalon, char *desc) {
    int i=0;
    while (i <= TAILLEMAX)
    {
        if (strcmp(tabSockets[i].nomChaine, nomSalon) == 0)
        {
            strcpy(tabSockets[i].descrChaine, desc);
            return 0;
        }
        i = i+10;   
    }
    return -1;
}

int deleteSalon(char *nomSalon) {
    int i=0;
    char msg[280];
    while (i <= TAILLEMAX)
    {
        if (strcmp(tabSockets[i].nomChaine, nomSalon) == 0)
        {
            for (int j = 0; j < 10; j++) {
                strcpy(tabSockets[i+j].nomChaine, "");
                strcpy(tabSockets[i+j].descrChaine, "");
                strcpy(msg, "quit");
                send(tabSockets[i+j].dSC, msg, sizeof(msg), 0);
            }
            return 0;
        }
        i = i+10;   
    }
    return -1;
}

int connexionSalon(int dSC, char *pseudo) {
    char msg[280];
    char nomSalon[30];
    int res = 0;
    while (1) {
        //Récupère le nom du salon
        recv(dSC, nomSalon, sizeof(nomSalon), 0);
         puts(nomSalon);
        if (strcmp(nomSalon, "0") == 0) {
            strcpy(msg, "0");
            send(dSC, msg, sizeof(msg), 0);
            return -1;
        }
        //get premiere place vide
        res = inserer(nomSalon, dSC, pseudo);
        if (res == 0) {
            printf("Insertion réussie\n");
            strcpy(msg, "0");
            send(dSC, msg, sizeof(msg), 0);
            break;
        } else if (res == -1) {
            printf("Nom du salon inexistant\n");
        } else {
            res = -1;
            printf("Salon plein\n");
        }
        strcpy(nomSalon,"");
        strcpy(msg, "-1");
        send(dSC, msg, sizeof(msg), 0);

    }
    return 0;
}

int creerSalon(int dSC) {
    char msg[280];
    char description[280];
    char nomSalon[30];
    int res = 0;
    while (1) {
        //Récupère le nom du salon
        recv(dSC, nomSalon, sizeof(nomSalon), 0);

        if (strcmp(nomSalon, "0") == 0) {
            strcpy(msg, "0");
            send(dSC, msg, sizeof(msg), 0);
            return -1;
        }
        if (salonExistant(nomSalon) == 0) {
            strcpy(msg, "0");
            send(dSC, msg, sizeof(msg), 0);
            recv(dSC, description, sizeof(description), 0);
            res = ajouterSalon(nomSalon, description);
            if (res == 0) {
                strcpy(msg, "0");
                send(dSC, msg, sizeof(msg), 0);
                break;
            } else {
                strcpy(msg, "-1");
                send(dSC, msg, sizeof(msg), 0);
            }
        } else {
            strcpy(msg, "-1");
            send(dSC, msg, sizeof(msg), 0);
        }        
    }
    return 0;
}

int editerSalon(int dSC) {
    char msg[280];
    char description[280];
    char nomSalon[30];
    int res = 0;
    while (1) {
        //Récupère le nom du salon
        recv(dSC, nomSalon, sizeof(nomSalon), 0);

        if (strcmp(nomSalon, "0") == 0) {
            strcpy(msg, "0");
            send(dSC, msg, sizeof(msg), 0);
            return -1;
        }
        if (salonExistant(nomSalon) == -1) {
            strcpy(msg, "0");
            send(dSC, msg, sizeof(msg), 0);
            recv(dSC, description, sizeof(description), 0);
            res = editDescSalon(nomSalon, description);
            if (res == 0) {
                strcpy(msg, "0");
                send(dSC, msg, sizeof(msg), 0);
                break;
            } else {
                strcpy(msg, "-1");
                send(dSC, msg, sizeof(msg), 0);
            }
        } else {
            strcpy(msg, "-1");
            send(dSC, msg, sizeof(msg), 0);
        }        
    }
    return 0;
}

int supprimerSalon(int dSC) {
    char msg[280];
    char nomSalon[30];
    int res = 0;
    while (1) {
        //Récupère le nom du salon
        recv(dSC, nomSalon, sizeof(nomSalon), 0);

        if (strcmp(nomSalon, "0") == 0) {
            strcpy(msg, "0");
            send(dSC, msg, sizeof(msg), 0);
            return -1;
        }
        if (salonExistant(nomSalon) == -1) {
            res = deleteSalon(nomSalon);
            if (res == 0) {
                strcpy(msg, "0");
                send(dSC, msg, sizeof(msg), 0);
                break;
            } else {
                strcpy(msg, "-1");
                send(dSC, msg, sizeof(msg), 0);
            }
        } else {
            strcpy(msg, "-1");
            send(dSC, msg, sizeof(msg), 0);
        }        
    }
    return 0;
}

/* Thread qui réceptionne le message d'un client et l'envoie à l'autre en boucle  */
void *threadEnvoi(void *args)
{
    char msg[280], chaines[2000];
    char messageComplet[280];
    char nomSalon[280];
    char action[3];
    int res, fin = 0, i=0;
    char pseudo[280];
    struct thread_args *argument = (struct thread_args*) args;
    int dSC = argument->dSC;
    pthread_t tid[TAILLEMAX];
    /* Réception du pseudo */
    while (fin != 1)
    {
        recv(dSC, pseudo, sizeof(pseudo), 0);
        /* Si le pseudo est déjà pris, on demande d'en rentrer un autre */
        if (getPseudo(pseudo) == 0)
        {
            strcpy(msg, "-1");
            printf("EXISTANT : %s\n", msg);
            send(dSC, msg, sizeof(msg), 0);
        }
        else
        {
            strcpy(msg, "0");
            printf("OK : %s\n", msg);
            send(dSC, msg, sizeof(msg), 0);
            fin = 1;
        }
        bzero(msg, 280);
    }
    
    while (1) {
        //Envoi les chaines disponibles
        strcpy(chaines, getChaines());
        printf("Chaines : %s\n", chaines);

        if (strcmp(chaines, "") == 0) {
            strcpy(chaines, "Aucune chaîne de disponible\n");
            send(dSC, chaines, sizeof(chaines), 0);
        } else {
            printf("Envoi des chaines au client\n");
            res = send(dSC, chaines, sizeof(chaines), 0);
	    printf("%d\n", res);
        }
        //Reçoit l'action 0 : Déconnexion, 1 : Connexion à un salon, 2: Créer un salon 3 : Edition d'un salon, 4 : Suppression d'un salon
        recv(dSC, action, sizeof(action), 0);

        if (strcmp(action, "0") == 0) {
            strcpy(msg, "0");
            //Envoi de la confirmation
            send(dSC, msg, sizeof(msg), 0);
            return NULL;
        } else if (strcmp(action, "1") == 0) {
            strcpy(msg, "0");
            //Envoi de la confirmation
            send(dSC, msg, sizeof(msg), 0);
            res = connexionSalon(dSC, pseudo);
            if (res == 0) {
                break;
            }
        } else if (strcmp(action, "2") == 0) {
            strcpy(msg, "0");
            send(dSC, msg, sizeof(msg), 0);
            creerSalon(dSC);
        } else if (strcmp(action, "3") == 0) {
            strcpy(msg, "0");
            send(dSC, msg, sizeof(msg), 0);
            editerSalon(dSC);
        } else if(strcmp(action, "4") == 0) {
            strcpy(msg, "0");
            send(dSC, msg, sizeof(msg), 0);
            supprimerSalon(dSC);
        } else {
            strcpy(msg, "-1");
            send(dSC, msg, sizeof(msg), 0);
        }

    }


    //Envoi 0 ou -1 selon si c'est possible

    printf("Pseudo : %s\n", pseudo);
    printf("ID du client : %d\n", dSC);
   // printf("Adresse IP : %s\n", inet_ntoa(tabSockets[i].aC.sin_addr));
   // printf("Port : %d\n", (int)ntohs(tabSockets[i].aC.sin_port));
    strcpy(nomSalon,getSalonByPseudo(pseudo));
    printf("NOM SALON : %s\n", nomSalon);
    /* Condition d'arrêt : Pas de réception (le client a mis fin à la connexion) */
    while (1)
    {
        /* En attente de réception d'un message */
        res = recv(dSC, msg, sizeof(msg), 0);
        if (res == 0)
        {
            break;
        }
        /* Si le message reçu est "fin" */
        if (strcmp(msg, "fin") == 0)
        {
            printf("MESSAGE FIN RECU\n");
            /* Formattage du message à envoyer */
            strcpy(messageComplet, pseudo);
            strcat(messageComplet, " a quitté la discussion\0\n");
            /* Envoi à tous les clients */
            while (i < TAILLEMAX)
            {
                if (strcmp(tabSockets[i].nomChaine, nomSalon) == 0) {
                    if (tabSockets[i].dSC != 0)
                    {
                        /* Suppression du socket du client ayant émit "fin" dans le tableau */
                        if (tabSockets[i].dSC == dSC)
                        {
                            tabSockets[i].dSC = 0;
                            strcpy(tabSockets[i].pseudo, "");
                        }
                        else
                        {
                            printf("ENVOI AU CLIENT %d\n", i);
                            res = send(tabSockets[i].dSC, messageComplet, sizeof(messageComplet), 0);
                            if (res == 0)
                            {
                                tabSockets[i].dSC = 0;
                            }
                        }
                    }
                }
                i += 1;
            }
            return NULL;
            /* Si le message reçu est "file" */
        }
        else if (strcmp(msg, "file") == 0)
        {
            pthread_create(&tid[i], NULL, threadEnvoiFichier, i);
        }
        else
        {
            /* Formattage du message à envoyer */
            strcpy(messageComplet, pseudo);
            strcat(messageComplet, " : ");
            strcat(messageComplet, msg);
            printf("%s\n", messageComplet);
            /* Envoi à tous les clients */
            i = 0;
            while (i < TAILLEMAX)
            {
                if (strcmp(tabSockets[i].nomChaine, nomSalon) == 0) {
                    if (tabSockets[i].dSC != 0 && tabSockets[i].dSC != dSC)
                    {
                        printf("ENVOI AU CLIENT %d\n", tabSockets[i].dSC);
                        res = send(tabSockets[i].dSC, messageComplet, sizeof(messageComplet), 0);
                        if (res == 0)
                        {
                            tabSockets[i].dSC = 0;
                        }
                    }
                } 
                i += 1;
            }
            bzero(msg, 280);
            bzero(messageComplet, 280);
        }
    }
    return NULL;
}

void *threadServeurFichier()
{
    int i = 0;
    dSFiles = socket(PF_INET, SOCK_STREAM, 0);
    struct sockaddr_in ad2;
    ad2.sin_family = AF_INET;
    ad2.sin_addr.s_addr = INADDR_ANY;
    ad2.sin_port = 35000;
    bind(dSFiles, (struct sockaddr *)&ad2, sizeof(ad2));
    printf("Initialisation du serveur de fichiers terminé \n");

    listen(dSFiles, 10);
    struct sockaddr_in aC2;
    socklen_t lg2 = sizeof(struct sockaddr_in);
    printf("EN ATTENTE FILES\n");

    /* Boucle principale pour les connexions des clients */
    while (1)
    {
        /* Attente de connexions entrantes */
        tabSockets[i].dSC2 = accept(dSFiles, (struct sockaddr *)&aC2, &lg2);
        tabSockets[i].aC2 = aC2;
        printf("Client FICHIER DSC : %d\n", tabSockets[i].dSC2);
        i++;
    }
}

int main(void)
{
    /* Déclaration des variables */
    int res, i = 0;
    pthread_t tid[TAILLEMAX];
    struct thread_args thr;

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
    pthread_t serveurFichier;
    pthread_create(&serveurFichier, NULL, threadServeurFichier, NULL);

    signal(SIGINT, traitement);
    /* Boucle principale pour les connexions des clients */
    while (1)
    {
        /* Attente de connexions entrantes */
        thr.dSC = accept(dS, (struct sockaddr *)&aC, &lg);
        thr.aC = aC;
        printf("Client DSC : %d\n", thr.dSC);

        /* Création du thread permettant la transmission des messages */
        pthread_create(&tid[i], NULL, threadEnvoi, (void *) &thr);
        i++;
    }

    /*Arrêt du serveur et fin */
    printf("Arrêt du serveur...\n");
    close(dS);
    return 0;
}

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

void vidermemoiretamponclavier(void)
{
    /*vide la mémoire tampon du clavier*/
    char c;
    while ((c = getchar()) != '\n' && c != EOF);
}

/* Structure des arguments passés au thread (client 1 et client 2)*/
struct thread_args {
    int dSC;
    int dSC2;
};

/* Thread qui réceptionne le message d'un client et l'envoie à l'autre en boucle  */
void *threadEnvoi(void *args) {
    char msg[280];
    int res;
    /* Récupération des arguments */
    struct thread_args *arguments = (struct thread_args *) args;
    /* Condition d'arrêt : Pas de réception (le client a mis fin à la connexion) */
    while (1) {
        res = recv(arguments->dSC, msg, sizeof(msg),0);
        if (res < 0) {
            break;
        }
        res = send(arguments->dSC2, msg, sizeof(msg), 0);
        if (res < 0) {
            break;
        }
        bzero(msg, 280);
    }
    return NULL;
}

int main(void)
{
    /* Déclaration des variables */
    int dS, res, deb = 0;
    pthread_t tid1, tid2;

    /* Déclaration des structures */
    struct thread_args arguments1;
    struct thread_args arguments2;

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
        /* Condition d'arrêt du serveur */
        if (deb == 1)
        {
            printf("Voulez vous arrêter le serveur ? o/n\n");
            if (getchar() == 'o')
            {
                break;
            }
            vidermemoiretamponclavier();
        }
        deb = 1;
        printf("En attente de connexion ...\n");

        /* Attente de 2 connexions entrantes */
        arguments1.dSC = accept(dS, (struct sockaddr *)&aC, &lg);
        arguments1.dSC2 = accept(dS, (struct sockaddr *)&aC, &lg);
        arguments2.dSC = arguments1.dSC2;
        arguments2.dSC2 = arguments1.dSC;

        /* Création de 2 threads permettant la transmission des messages */
        pthread_create(&tid1, NULL, threadEnvoi, (void *)&arguments1);
        pthread_create(&tid2, NULL, threadEnvoi, (void *)&arguments2);
        /* Attends que les threads soient terminés */
        pthread_join(tid1, NULL);
        pthread_join(tid2, NULL);

        pthread_exit(NULL);

    }

    /*Arrêt du serveur et fin */
    printf("Arrêt du serveur...\n");
    close(dS);
    return 0;
}

#include <stdio.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


int connection (int sock){
    /* demande à l'utilisateur de rentrer l'addresse ip et le port du serveur
    et se connecte au serveur
    prend en entré une socket en ipv4, de type TCP
    renvoie 0 si tout s'est bien passé, et -1 si il y a eu un problème*/

    char IP[INET_ADDRSTRLEN];
    printf ("Veuillez entrer l'adresse IP du serveur:\n");
    fgets (IP,sizeof(IP),stdin);

    int port;
    printf("Veuillez entrer le port du serveur:\n");
    scanf("%d", &port);


    struct sockaddr_in adServ;
    adServ.sin_family = AF_INET;
    adServ.sin_port = port;
    int res = inet_pton(AF_INET,IP, &(adServ.sin_addr));
    if (res == 0){
        printf("Erreur: L'adresse IP entrée n'est pas valide\n");
        return -1;
    }
    socklen_t lgA = sizeof(struct sockaddr_in);
    res = connect(sock,(struct sockaddr *) &adServ,lgA);
    if (res == -1){
        printf("Erreur: Le serveur n'est pas accessible\n");
        return -1;
    }

    return 0;
}


int main (void){

    char reponse[2];
    bool emetteur;
    printf("Tapez 1 si vous voulez émettre, 0 si vous voulez recevoir:\n");
    fgets(reponse,sizeof(reponse) ,stdin);
    if (strcmp(reponse,"1")==0){
        emetteur = true;
    }
    else{
        emetteur = false;
    }
    printf("Vous êtes ");
    if (emetteur){
        printf("émetteur\n");
    }
    else{
        printf("recepteur\n");
    }

    int dSock = socket (PF_INET, SOCK_STREAM, 0);
    int res = connection(dSock);
    if (res == -1){
        printf("Erreur: La connection a échoué\n");
        return -1;
    }

    bool continuer = true;
    char message[280];

    while(continuer){
        if (emetteur){
            printf("Tapez le message en 280 caractères:\n");
            fgets(message, 280, stdin);
            char * pos1 = strchr(message,'\n');
            *pos1 ='\0';

            res = send(dSock,message, strlen(message)+1,0);
            if (res == -1){
                printf("Erreur: Le message n'a pas été envoyé\n");
                break;
            }
            if (strcmp(message,"fin")==0){
                continuer = false;
            }
            emetteur = false;
        }

        else{
            printf("En attente de message\n");
            res = recv(dSock, message, sizeof(message),0);
            if (res == 0){
                printf("Warning: Serveur déconnecté\n");
                break;
            }
            else if(res == -1){
                printf("Erreur: Pas de message reçus\n");
                break;
            }
            printf("Reçu: \n%s\n", message);
            if (strcmp(message,"fin") == 0){
                continuer = false;
            }
            emetteur = true;
        }
    }
    close(dSock);
    printf("Déconnection\n");
    return 0;
}

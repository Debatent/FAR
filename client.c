#include <stdio.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

void vidermemoiretamponclavier(void){
    /*vide la mémoire tampon du clavier*/
    char c;
    while( (c=getchar())!='\n' && c!=EOF);
}





int connection (int sock){
    /* demande à l'utilisateur de rentrer l'addresse ip et le port du serveur
    et se connecte au serveur
    prend en entré une socket en ipv4, de type TCP
    renvoie 0 si tout s'est bien passé, et -1 si il y a eu un problème*/
    char ip[INET_ADDRSTRLEN];/*Se connect à l'adresse ip*/
    printf ("Veuillez entrer l'adresse IP du serveur:\n");
    fgets (ip,sizeof(ip),stdin);
    char * correction = strchr(ip,'\n');
    *correction ='\0';

    /*printf ("%s\n", ip);*/

    int port;
    printf("Veuillez entrer le port du serveur:\n");
    scanf("%d", &port);
    vidermemoiretamponclavier();
    /*printf("%d\n",port);*/

    struct sockaddr_in adServ;
    adServ.sin_family = AF_INET;
    adServ.sin_port = port;
    int res = inet_pton(AF_INET,ip, &(adServ.sin_addr));
    if (res == 0){
        printf("Erreur: L'adresse IP entrée n'est pas valide\n");
        return -1;
    }

    printf("Tentative de connection\n");
    socklen_t lgA = sizeof(struct sockaddr_in);
    res = connect(sock,(struct sockaddr *) &adServ,lgA);
    if (res == -1){
        printf("Erreur: Le serveur n'est pas accessible\n");
        return -1;
    }
    else{
        printf("Serveur trouvé\n");
    }

    return 0;
}








int main (void){

    /*choix émission ou reception initial*/
    bool emetteur;
    char reponse;
    printf("Tapez 1 si vous voulez émettre, 0 si vous voulez recevoir (réception par défaut):\n");
    reponse = getchar();
    vidermemoiretamponclavier();

    if (reponse == '1'){
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

    /* connection au serveur*/
    int dSock = socket (PF_INET, SOCK_STREAM, 0);
    int res = connection(dSock);
    if (res == -1){
        printf("Erreur: La connection a échoué\n");
        return -1;
    }
    else{
        printf("Connection réussie\n");
    }
    char msg[2] = "1";
    char msg2[2] = "0";

    /*permet de configurer le serveur si emetteur ou recepteur*/
    if (emetteur){
        send(dSock,msg, sizeof(msg),0);
    }
    else{
        send(dSock,msg2, sizeof(msg2),0);
    }

    bool continuer = true;
    int nbrdecaractere = 280;
    char message[nbrdecaractere+1];
    while(continuer){
        if (emetteur){/*pour émettre un message*/
            printf("Tapez le message en %d caractères:\n", nbrdecaractere);
            fgets(message, nbrdecaractere+1, stdin);

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
            printf("En attente de message\n");/*Pour recevoir un message*/
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

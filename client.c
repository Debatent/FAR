#include <stdio.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>

struct message{
    int taille;
    char * msg;
};

struct messagethread{
    int dSock;
    struct message message;
};



void vidermemoiretamponclavier(void){
    /*vide la mémoire tampon du clavier*/
    char c;
    while( (c=getchar())!='\n' && c!=EOF);
}





int connexion (int sock){
    /* demande à l'utilisateur de rentrer l'addresse ip et le port du serveur
    et se connecte au serveur
    prend en entrée une socket en ipv4, de type TCP
    renvoie 0 si tout s'est bien passé, et -1 si il y a eu un problème*/
    char ip[INET_ADDRSTRLEN];/*Se connecte à l'adresse ip*/
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

    printf("Tentative de connexion\n");
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



void *recevoirmessage(void* args){
    struct messagethread *argument = (struct messagethread*) args;
    int res;
    char msg[argument->message.taille];
    while(1){
        res = recv(argument ->dSock,msg, sizeof(msg),0);
        if (res == 0){
            printf("Warning: Serveur déconnecté\n");
            break;
        }
        else if(res == -1){
            printf("Erreur: Pas de message reçus\n");
            break;
        } else if (strcmp(msg, "fin") == 0) {
            printf("Fin de la réception des messages\n");
            break;
        }
        puts(msg);
    }
    return NULL;
}


void *envoyermessage(void* args){
    struct messagethread *argument = (struct messagethread*) args;
    int res;
    char msg [argument->message.taille];
    while(1){
        fgets(msg, argument->message.taille + 1, stdin);

        char * pos1 = strchr(msg,'\n');
        *pos1 ='\0';

        res = send(argument->dSock,msg, strlen(msg)+1,0);
        if (res == -1){
            printf("Erreur: Le message n'a pas été envoyé\n");
            break;
        }
        if (strcmp(msg,"fin")==0){
            printf("Fin de l'envoi des messages\n");
            break;
        }
    }
    return NULL;
}





int main (void){

    /* connexion au serveur*/
    int dSock = socket (PF_INET, SOCK_STREAM, 0);
    int res = connexion(dSock);
    if (res == -1){
        printf("Erreur: La connexion a échouée\n");
        return -1;
    }
    else{
        printf("Connexion réussie\n");
    }




    struct message message;
        message.taille = 280;



    struct messagethread msgthr;
        msgthr.dSock = dSock;
        msgthr.message = message;

    pthread_t recepteur;
    pthread_create (&recepteur, NULL, recevoirmessage, (void *)&msgthr);

    printf("Tapez les messages en %d caractères:\n", message.taille);
    pthread_t envoyeur;
    pthread_create (&envoyeur, NULL, envoyermessage, (void *)&msgthr);

    printf("PTHREAD JOIN\n");

    pthread_join(recepteur, NULL);
    pthread_join(envoyeur,NULL);

    close(dSock);

    printf("Déconnexion\n");

    return 0;
}

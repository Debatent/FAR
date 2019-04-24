#include <stdio.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdbool.h>
#include <pthread.h>

/*Information à transmettre au thread*/
struct messagethread{
    int dSock;
    int taillemsg;
    int taillepseudo;
};



void vidermemoiretamponclavier(void){
    /*vide la mémoire tampon du clavier*/
    char c;
    while( (c=getchar())!='\n' && c!=EOF);
}





int connexion (int sock){
    /**
     *demande à l'utilisateur de rentrer l'addresse ip et le port du serveur
     *et se connecte au serveur
     *prend en entrée une socket en ipv4, de type TCP
     *renvoie 0 si tout s'est bien passé, et -1 si il y a eu un problème
     */

    /*Saisi de l'adresse IP du serveur*/
    char ip[INET_ADDRSTRLEN];
    printf ("Veuillez entrer l'adresse IP du serveur:\n");
    fgets (ip,sizeof(ip),stdin);
    char * correction = strchr(ip,'\n');
    *correction ='\0';

    /*printf ("%s\n", ip);*/

    /*Saisie du port du serveur*/
    int port;
    printf("Veuillez entrer le port du serveur:\n");
    scanf("%d", &port);
    vidermemoiretamponclavier();
    /*printf("%d\n",port);*/

    /*Convertion de l'addresse IP en binaire*/
    struct sockaddr_in adServ;
    adServ.sin_family = AF_INET;
    adServ.sin_port = port;
    int res = inet_pton(AF_INET,ip, &(adServ.sin_addr));
    if (res == 0){
        printf("Erreur: L'adresse IP entrée n'est pas valide\n");
        return -1;
    }

    /*Connexion au serveur*/
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


int entrerpseudo(void* args){
    /**
     *Demande à l'utilisateur son pseudo, et le redemande si le pseudo est déjà pris
     *Prend en entrée un message thread
     *renvoie 0 si le pseudo a été validé, -1 en cas d'erreur
     */
     struct messagethread *argument = (struct messagethread*) args;
     char pseudonyme[argument->taillepseudo];
     char reponse[3q];
     int res;
     while(1){
         printf("Veuillez entrer votre pseudo en %d caractères maximum:\n",argument->taillepseudo);
         fgets(pseudonyme, argument->taillepseudo + 1, stdin);

         char * pos1 = strchr(pseudonyme,'\n');
         *pos1 ='\0';

         res = send (argument->dSock, pseudonyme,strlen(pseudonyme) + 1,0);
         if (res<0){
             return -1;
         }
         res = recv(argument->dSock, reponse,strlen(reponse) + 1,0);
         if (res<=0){
             return-1;
         }
         else if (strcmp(reponse, "-1\0")){
             puts("Pseudo déjà pris par quelqu'un d'autre");
         }
         else{
             printf("Bienvenue %s\n",pseudonyme);
             break;
         }
     }
     return 0;
}


void* envoyerdestinataire(void* args){
    /**
     *Demande et envoie le pseudo du recepteur
     */
    struct messagethread *argument = (struct messagethread*) args;
    char pseudo[argument->taillepseudo];
    printf("Entrez le pseudo du destinataire");
    fgets(pseudonyme, argument->taillepseudo, stdin);
    send (argument->dSock,pseudonyme, sizeof(pseudonyme),0 );


}

void* envoyerfichier(void* args){
    /**
     *affiche les nom des fichiers du répertoire files
     *Demande à l'émetteur le nom du fichier
     *envoie le titre puis le contenu du fichier selectionné au recepteur
     */
}


void* recevoirfichier(void* args){
    /**
     *reçoit le titre puis le contenu du fichier qu'on place dans le repertoire
     *reception
     */
}



void *recevoirmessage(void* args){
    /**
     *Fonction pour le thread
     *Recois les messages en TCP du serveur et les affiche à l'utilisateur
     *Si on recois le message "fin" ou
     *si il y a une erreur de reception->sort de la boucle
     */
    struct messagethread *argument = (struct messagethread*) args;
    int res, dSock;
    char msg[280];
    dSock = argument->dSock;

    while(1){
        res = recv(dSock,msg, sizeof(msg)+1,0);
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
        printf("%s\n", msg);
        bzero(msg, 280);
    }
    return NULL;
}


void *envoyermessage(void* args){
    /**
     *Fonction pour le thread
     *Saisi et envoie les messages en TCP au serveur
     *Si on envoie le message "fin" ou
     *si il y a une erreur d'envoie->sort de la boucle
     */
    struct messagethread *argument = (struct messagethread*) args;
    int res;
    char msg [argument->taillemsg-1];
    int dSock = argument->dSock;
    while(1){
        fgets(msg, argument->taillemsg-1, stdin);

        char * pos1 = strchr(msg,'\n');
        *pos1 ='\0';

        res = send(dSock,msg, sizeof(msg)+1,0);
        if (res == -1){
            printf("Erreur: Le message n'a pas été envoyé\n");
            break;
        }
        if (strcmp(msg,"fin")==0){
            printf("Fin de l'envoi des messages\n");
            break;
        }
        else if (strcmp(msg,"file")){

        }
    }
    return NULL;
}





int main (void){

    /*Création de la socket*/
    int dSock = socket (PF_INET, SOCK_STREAM, 0);
    /* connexion au serveur*/
    int res = connexion(dSock);
    if (res == -1){
        printf("Erreur: La connexion a échouée\n");
        return -1;
    }
    else{
        printf("Connexion réussie\n");
    }




    /*Instance des informations du thread*/
    struct messagethread msgthr;
        msgthr.dSock = dSock;
        msgthr.taillemsg = 280;
        msgthr.taillepseudo = 12;

    /*Saisie du pseudo et envoie au Serveur*/
    res = entrerpseudo((void*)&msgthr.taillepseudo);
    if (res<0){
        puts("Erreur de communication avec le serveur");
        close(dSock);
        puts("Arret");
        return 0;
    }

    /*Création des threads d'envoi et de reception*/
    pthread_t recepteur;
    pthread_create (&recepteur, NULL, recevoirmessage, (void *)&msgthr);

    printf("Tapez les messages en %d caractères:\n", msgthr.taillemsg);
    pthread_t envoyeur;
    pthread_create (&envoyeur, NULL, envoyermessage, (void *)&msgthr);

    /*Attente de l'extinction du thread*/
    pthread_join(envoyeur,NULL);

    /*Fermeture de la connexion*/
    close(dSock);

    printf("Déconnexion\n");

    return 0;
}

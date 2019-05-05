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


//Bibliothèque servant à la sélection fichier

#include <limits.h>
#include <dirent.h>


//Information à transmettre aux threads de message*
struct messagethread{
    int dSock;
    int taillemsg;
    int taillepseudo;
};

//Information à transmettre aux threads de fichier
struct fichierthread{
    int SockServeurFichier;
    int SockEmetteur;
    int SockRecepteur;
    char ip[INET_ADDRSTRLEN];
    int port;
    int tailletransfert;
    char nomfichier[1023];
};

//IP du serveur
char ip[INET_ADDRSTRLEN];


//Envoie les signaux pour activer l'accès au clavier des différents threads
pthread_cond_t cond_activation_tranfert_fichier , cond_activation_message;



//Mutex servant à gérer l'accès au clavier
pthread_mutex_t clavier = PTHREAD_MUTEX_INITIALIZER;






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

    //Saisi de l'adresse IP du serveur
    printf ("Veuillez entrer l'adresse IP du serveur:\n");
    fgets (ip,sizeof(ip),stdin);
    char * correction = strchr(ip,'\n');
    *correction ='\0';

    //printf ("%s\n", ip);

    //Saisie du port du serveur*
    int port;
    printf("Veuillez entrer le port du serveur:\n");
    scanf("%d", &port);
    vidermemoiretamponclavier();
    //printf("%d\n",port);

    //Convertion de l'addresse IP en binaire
    struct sockaddr_in adServ;
    adServ.sin_family = AF_INET;
    adServ.sin_port = port;
    int res = inet_pton(AF_INET,ip, &(adServ.sin_addr));
    if (res == 0){
        printf("Erreur: L'adresse IP entrée n'est pas valide\n");
        return -1;
    }

    //Connexion au serveur
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












int entrerpseudo(struct messagethread argument){
    /**
     *Demande à l'utilisateur son pseudo, et le redemande si le pseudo est déjà pris
     *Prend en entrée un message thread
     *renvoie 0 si le pseudo a été validé, -1 en cas d'erreur
     */
     char pseudonyme[argument.taillepseudo];
     char reponse[280];
     int res;
     //On fait une boucle tant que le pseudo n'a pas déjà était pris par quelqu'un d'autre
     while(1){
         printf("Veuillez entrer votre pseudo en %d caractères maximum:\n",argument.taillepseudo);
         fgets(pseudonyme, argument.taillepseudo, stdin);

         char * pos1 = strchr(pseudonyme,'\n');
         *pos1 ='\0';

         res = send (argument.dSock, pseudonyme,strlen(pseudonyme) + 1,0);
         if (res<0){
             return -1;
         }
         res = recv(argument.dSock, reponse,strlen(reponse) + 1,0);
         printf("Réponse : %s\n", reponse);
         if (res<=0){
             return-1;
         }
         //Bug : Ne marche que la première fois
         else if ((strcmp(reponse, "-1")==0)){
             puts("Pseudo déjà pris par quelqu'un d'autre");
         }
         else{
             printf("Bienvenue %s\n",pseudonyme);
             break;
         }
         bzero(pseudonyme, argument.taillepseudo);
         bzero(reponse, 280);
     }
     return 0;
}














int envoyerdestinataire(struct fichierthread args){
    /**
     *Demande et envoie le pseudo du recepteur
     */
    int taillepseudo = 280;
    char pseudo[taillepseudo];
    printf("Entrez le pseudo du destinataire\n");
    fgets(pseudo, taillepseudo, stdin);
    send (args.SockServeurFichier,pseudo, sizeof(pseudo),0);

    return 0;


}














int get_last_tty() {
  FILE *fp;
  char path[1035];
  fp = popen("/bin/ls /dev/pts", "r");
  if (fp == NULL) {
    printf("Impossible d'exécuter la commande\n" );
    exit(1);
  }
  int i = INT_MIN;
  while (fgets(path, sizeof(path)-1, fp) != NULL) {
    if(strcmp(path,"ptmx")!=0){
      int tty = atoi(path);
      if(tty > i) i = tty;
    }
  }

  pclose(fp);
  return i;
}













FILE* new_tty() {
  pthread_mutex_t the_mutex;
  pthread_mutex_init(&the_mutex,0);
  pthread_mutex_lock(&the_mutex);
  system("gnome-terminal"); sleep(1);
  char *tty_name = ttyname(STDIN_FILENO);
  int ltty = get_last_tty();
  char str[2];
  sprintf(str,"%d",ltty);
  tty_name[strlen(tty_name)-1] = '\0';
  strcat(tty_name,str);
  FILE *fp = fopen(tty_name,"wb+");
  pthread_mutex_unlock(&the_mutex);
  pthread_mutex_destroy(&the_mutex);
  return fp;
}
















int choisirfichier(char nomdufichier[1023]){
    /**
     *demande le fichier
     *On appuie sur q pour arreter le choix
     *renvoie 0 si on peut l'ouvrir et -1 si on a appuyé sur q
     */
     FILE* fp1 = new_tty();
     fprintf(fp1,"%s\n","Ce terminal sera utilisé uniquement pour l'affichage");

     // Demander à l'utilisateur quel fichier afficher
     DIR *dp;
     struct dirent *ep;
     char fichier[2000] = "./Envoie";

     dp = opendir (fichier);
     if (dp != NULL) {
       fprintf(fp1,"Voilà la liste de fichiers du répertoire 'Envoie' :\n");
       while ((ep = readdir (dp))) {
         if(strcmp(ep->d_name,".")!=0 && strcmp(ep->d_name,"..")!=0)
       fprintf(fp1,"%s\n",ep->d_name);
       }
       (void) closedir (dp);
     }
     else {
       perror ("Ne peux pas ouvrir le répertoire");
     }

     //On fait une boucle tant que l'utilisateur n'a pas entré le bon fichier ou tapé "q"
     bool choixcorrecte = false;
     char fileName[1023];
     //Nom complet du chemin jusqu'au fichier
     char cheminfinal[2000]="";
     while(!choixcorrecte){
         strcat(cheminfinal,fichier);
         printf("Indiquez le nom du fichier : ");
         fgets(fileName,sizeof(fileName),stdin);
         fileName[strlen(fileName)-1]='\0';
         strcat(cheminfinal,fileName);

         FILE *fps = fopen(cheminfinal, "r");

         //on a rentré q: sortie de cette fonction
         if (strcmp(fileName,"q") == 0){
             puts("Sortie");
             return -1;
         }
         //Nom de fichier incorrecte
         else if (fps == NULL){
             printf("Ne peux pas ouvrir le fichier suivant : %s",fileName);
             strcpy(cheminfinal,"");
         }
         //Nom de fichier correcte
         else {
             puts("fichier correcte");
             choixcorrecte = true;
             fclose(fps);
         }
     }
     //change la valeur en parametre
     strcpy(nomdufichier,fileName);
     return 0;
}

















void* envoiefichier(void* args){
    /**
     *Envoie le nom du fichier puis le contenu du fichier par packet de 1000
     */
    struct fichierthread *argument = (struct fichierthread*) args;
    char str[argument -> tailletransfert];
    char fichier[2000] = "./Envoie";
    strcat(fichier,argument -> nomfichier);

    FILE *fps = fopen(fichier, "r");
    send(argument->SockRecepteur, argument->nomfichier, strlen(argument->nomfichier),0);

    // Lire et envoyer le contenu du fichier
    while (fgets(str, 1000, fps) != NULL) {
        send(argument->SockRecepteur, str, strlen(str),0);
    }
    printf("Fichier %s envoyé\n",argument->nomfichier);
    close(argument->SockRecepteur);
    pthread_exit(0);
}





















void* recoisfichier(void* args){
    /**
     *créée le fichier dans le répertoire Reception et le remplie
     */
    struct fichierthread *argument = (struct fichierthread*) args;
    //Nom complet du chemin
    char fichier[2000] = "./Reception/";
    recv(argument ->SockEmetteur, argument -> nomfichier, sizeof(argument -> nomfichier),0);
    strcat(fichier,argument -> nomfichier);
    //On ouvre/crée le ficjier en mode ajout à la fin
    FILE *fps = fopen(fichier, "a");
    char element[argument -> tailletransfert];
    //Tant que l'emetteur n'a pas coupé la communication, on ajoute des elements dans le fichier
    while ((recv(argument ->SockEmetteur,element, sizeof(element),0)>0)){
        fputs(element,fps);
    }

    fclose(fps);
    printf("Fichier %s reçus", argument->nomfichier);

    close(argument -> SockEmetteur);
    pthread_exit(0);
}















void* gestionenvoyerfichier(void* args){
    /**
    *affiche les nom des fichiers du répertoire files
    *Demande à l'émetteur le nom du fichier
    *envoie le titre puis le contenu du fichier selectionné au recepteur
    */
    struct fichierthread *argument = (struct fichierthread*) args;

    int dS = socket(PF_INET, SOCK_STREAM, 0);
    struct sockaddr_in ad;
    ad.sin_family = AF_INET;
    ad.sin_addr.s_addr = INADDR_ANY;
    ad.sin_port = 45000;
    bind(dS, (struct sockaddr *)&ad, sizeof(ad));


    listen(dS, 10);
    struct sockaddr_in aC;
    socklen_t lg = sizeof(struct sockaddr_in);

    argument -> SockEmetteur = dS;
    int res;
    while(true){
        //on attend que l'utilisateur tape file pour entrer ici
        pthread_cond_wait(&cond_activation_tranfert_fichier,&clavier);
        pthread_mutex_lock(&clavier);
        envoyerdestinataire(*argument);

        char fileName[1023];
        res = choisirfichier(fileName);
        pthread_mutex_unlock(&clavier);
        pthread_cond_signal(&cond_activation_message);

        if (res == 0){
            strcpy(argument->nomfichier, fileName);
            argument->SockRecepteur = accept(dS, (struct sockaddr *)&aC, &lg);
            //On créé un thread pour envoyer le fichier à la personne qui vient de se connecter
            pthread_t tid;
            pthread_create(&tid, NULL, envoiefichier,(void*) argument);
        }

    }

    pthread_exit(0);


}














void* gestionrecevoirfichier(void* args){
    /**
    *Recois les informations de l'émetteur via le serveur et se connecte directement
    *à lui
    *Crée le thread dédié à la reception du fichier
    */
    struct fichierthread *argument = (struct fichierthread*) args;

    int dSockrecepteur = socket (PF_INET, SOCK_STREAM, 0);

    //Reception information emetteur

    while(true){
        // Le serveur envoie d'abord l'IP de l'emetteur
        recv(argument->SockServeurFichier, argument -> ip, sizeof( argument -> ip),0);
        //Le serveur envoie ensuite le port de l'emetteur
        recv(argument->SockServeurFichier, &argument -> port, sizeof(int),0);

        //On configure le socket du recepteur pour qu'il puisse se connecter à l'emetteur
        struct sockaddr_in adEmet;
        adEmet.sin_family = AF_INET;
        adEmet.sin_port = argument -> port;
        int res = inet_pton(AF_INET, argument -> ip, &(adEmet.sin_addr));
        if (res == 0){
            printf("Erreur: L'adresse IP entrée n'est pas valide\n");
        }

        //Connexion à l'emetteur
        socklen_t lgA = sizeof(struct sockaddr_in);
        res = connect(dSockrecepteur,(struct sockaddr *) &adEmet,lgA);
        if (res == -1){
            printf("Erreur: L'émetteur n'est pas accessible\n");
        }
        argument->SockRecepteur = dSockrecepteur;
        //On crée le thread dédié à la reception de ce fichier
        pthread_t tid;
        pthread_create( &tid, NULL, recoisfichier,(void*) argument);
    }
    pthread_exit(0);
}




















void* gestionfichier(void* args){
    /**
     *Se connecte au serveur, et gère les transfert de fichiers
     *(selectionner le fichier à envoyer, le nom du destinatiare, transfert du fichier, reception du fichier)
     */

    int dSockserveur = socket (PF_INET, SOCK_STREAM, 0);

    struct sockaddr_in adServ;
    adServ.sin_family = AF_INET;
    adServ.sin_port = 35000;
    int res = inet_pton(AF_INET,ip, &(adServ.sin_addr));
    if (res == 0){
        printf("Erreur: L'adresse IP entrée n'est pas valide\n");
        _exit(0);
    }

    //Connexion au serveur de fichier
    socklen_t lgA = sizeof(struct sockaddr_in);
    res = connect(dSockserveur,(struct sockaddr *) &adServ,lgA);
    if (res == -1){
        printf("Erreur: Le serveur de fichier n'est pas accessible\n");
        _exit(0);
    }

    struct fichierthread fchthr;
        fchthr.SockServeurFichier = dSockserveur;
        fchthr.tailletransfert = 1000;


    pthread_t gestionemission;
    pthread_t gestionreception;

    pthread_create(&gestionemission, NULL, gestionenvoyerfichier, (void*) &fchthr);

    pthread_create(&gestionreception, NULL, gestionrecevoirfichier, (void*) &fchthr);

    pthread_join(gestionemission, NULL);

    pthread_join(gestionreception, NULL);

    pthread_exit(0);
}




























void *envoyermessage(void* args){
    /**
     *Fonction pour le thread
     *Saisi et envoie les messages en TCP au serveur
     *Si on envoie le message "fin" ou
     *si il y a une erreur d'envoie->sort de la boucle
     */
    struct messagethread *argument = (struct messagethread*) args;
    int res, port;
    char msg [argument->taillemsg-1];
    int dSock = argument->dSock;
    pthread_mutex_lock(&clavier);
    while(1){
        printf("Entrez un message : \n");
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
        else if (strcmp(msg,"ile") == 0){
            printf("Entrez le pseudo : \n");
            fgets(msg, argument->taillemsg-1, stdin);
            char * pos2 = strchr(msg,'\n');
            *pos2 ='\0';
            send(dSock, msg, sizeof(msg), 0);

            printf("Réception du port : ");
            recv(dSock, msg, sizeof(msg), 0);
            port = atoi(msg);
            printf("%d\n", port);
            bzero(msg, argument->taillemsg-1);
            printf("Réception de l'adresse IP : ");
            recv(dSock, msg, sizeof(msg), 0);
            printf("%s\n", msg);
            // ---------------------------------------------------- CODE CLIENT EMETTEUR DU FICHIER A CONTINUER ICI -------------------------------
            /*pthread_mutex_unlock(&clavier);
            pthread_cond_signal(&cond_activation_tranfert_fichier);
            pthread_cond_wait(&cond_activation_message,&clavier);
            pthread_mutex_lock(&clavier);*/
        }
        bzero(msg, argument->taillemsg-1);
    }
    pthread_exit(0);
}
















void *recevoirmessage(void* args){
    /**
     *Fonction pour le thread
     *Recois les messages en TCP du serveur et les affiche à l'utilisateur
     *Si on recois le message "fin" ou
     *si il y a une erreur de reception->sort de la boucle
     */
    struct messagethread *argument = (struct messagethread*) args;
    int res, dSock, port, ip;
    char msg[280];
    dSock = argument->dSock;

    while(1){
        res = recv(dSock,msg, sizeof(msg),0);
        if (res == 0){
            printf("Warning: Serveur déconnecté\n");
            break;
        }
        else if(res == -1){
            printf("Erreur: Pas de message reçus\n");
            break;
        } else if (strcmp(msg, "ile") == 0) {
            printf("Réception du port : ");
            recv(dSock, msg, sizeof(msg), 0);
            port = atoi(msg);
            printf("%d\n", port);
            printf("Réception de l'adresse IP : ");
            recv(dSock, msg, sizeof(msg), 0);
            printf("%s\n", msg);
            // --------------------------------- CODE CLIENT RECEPTEUR DU FICHIER A CONTINUER ICI ------------------------------
        } else {
            printf("%s\n", msg);
            bzero(msg, 280);
        }

    }
    pthread_exit(0);
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
    res = entrerpseudo(msgthr);
    if (res<0){
        puts("Erreur de communication avec le serveur");
        close(dSock);
        puts("Arret");
        return 0;
    }

    /*Initialisation des condition(pour la synchronisation)*/
    pthread_cond_init(&cond_activation_tranfert_fichier,0);

    pthread_cond_init(&cond_activation_message,0);


    /*Création des threads d'envoi et de reception*/
    pthread_t recepteur;
    pthread_create (&recepteur, NULL, recevoirmessage, (void *)&msgthr);

    printf("Tapez les messages en %d caractères:\n", msgthr.taillemsg);
    pthread_t envoyeur;
    pthread_create (&envoyeur, NULL, envoyermessage, (void *)&msgthr);
/*

    pthread_t fichier;
    pthread_create(&fichier, NULL, gestionfichier,&msgthr);

    Attente de l'extinction du thread*/
    pthread_join(envoyeur,NULL);

    /*Fermeture de la connexion*/
    close(dSock);

    printf("Déconnexion\n");

    return 0;
}

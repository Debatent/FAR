#include <stdio.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>


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

    int dSock = socket (PF_INET, SOCK_STREAM, 0);
    int res = connection(dSock);
    if (res == -1){
        printf("Erreur: La connection a échoué");
        return -1;
    }
    return 0;
}

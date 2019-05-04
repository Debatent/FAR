all:client serveur

# Créer un fichier "client" exécutable
client: client.c
	gcc -Wall -std=c99 -pthread -o client client.c

# Créer un fichier "serveur" exécutable
serveur: serveur.c
	gcc -Wall -std=c99 -pthread -o serveur serveur.c

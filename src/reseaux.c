/*
	Par Andréas Guillot
	et Ervin Altintas
*/

#include "../include/reseaux.h"

void clientHandshake(int socketId, 
					struct sockaddr_in * distantAddress)
{
	/*
		L'idée est qu'on va envoyer des messages
		tant qu'on ne peut pas en recevoir un.
		SYN -> SYNACK TCP
	*/

	printf("Début de la connexion côté client.\n");
	socklen_t tailleSocket = sizeof(*distantAddress);
	struct message m;

	/*
		Pour être sûr que les messages de connexion
		ne soient pas mal interpétés
	*/
	m.ack = m.seq = NOPE;

	while ((attente(socketId, TV_SEC, TV_USEC)) == 0)
	{
		printf("Envoi d'un paquet de demande de connexion.\n");
		if ((sendto(socketId, 
				&m, 
				sizeof(struct message) - BUFFER_LENGTH,
				0, 
				(struct sockaddr*)distantAddress, 
				tailleSocket)) == -1)
		{
			perror("sendto");
			exit(1);
		}
	}

	printf("Paquet de confirmation reçu.\n");

	if ((recvfrom(socketId, 
					&m, 
					sizeof(struct message) - BUFFER_LENGTH,
					0, 
					(struct sockaddr *)distantAddress, 
					&tailleSocket)) == -1)
	{
		perror("recvfrom");
		exit(1);
	}


	printf("Fin de la connexion côté client.\n");
}

void serverHandshake(int socketId, 
					struct sockaddr_in * distantAddress)
{
	/*
		SYN SYNACK
	*/

	printf("Début de la connexion côté serveur.\n");
	socklen_t tailleSocket = sizeof(*distantAddress);
	struct message m;
	m.ack = m.seq = NOPE;

	if ((recvfrom(socketId, 
					&m, 
					sizeof(struct message) - BUFFER_LENGTH,
					0, 
					(struct sockaddr *)distantAddress, 
					&tailleSocket)) == -1)
	{
		perror("recvfrom");
		exit(1);
	}
	printf("Réception d'une demande de connexion.\n");

	while ((attente(socketId, TV_SEC, TV_USEC)) == 0)
	{
		printf("Envoi d'un paquet de demande de connexion.\n");
		if ((sendto(socketId, 
				&m, 
				sizeof(struct message) - BUFFER_LENGTH,
				0, 
				(struct sockaddr*)distantAddress, 
				tailleSocket)) == -1)
		{
			perror("sendto");
			exit(1);
		}
	}

	printf("Fin de la connexion côté serveur.\n");
}

int attente(int socketId, int sec, int usec)
{
	fd_set set;
	struct timeval tv;
	int maxDescriptor;
	int retval;

	FD_ZERO(&set);
	FD_SET(socketId, &set);
	maxDescriptor = socketId + 1;

	tv.tv_sec = sec;
	tv.tv_usec = usec;

	if ((retval = select(maxDescriptor, &set, NULL, NULL, &tv)) < 0)
	{
		fprintf(stderr, "Erreur du select!\n");
		perror("select");
		exit(1);
	}

	return retval;
}

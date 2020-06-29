/*
	Par Andréas Guillot
	et Ervin Altintas
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <string.h>
#include "../include/reseaux.h"

int main(int argc, char *argv[])
{
	if ((argc < 5) || (argc > 6))
	{
		fprintf(stderr, "Argument 1 : Ficher envoyé / "
						"Argument 2 : Ficher reçu / "
						"Argument 3 : Adresse IP distante / "
						"Argument 4 : Port distant / "
						"[Argument 5 : Port local]\n");
		exit(1);
	}
	printf("BUFFER_LENGTH = %d\n", BUFFER_LENGTH);
	printf("sizeof(struct message) = %lu\n", sizeof(struct message));
	
	/* Variables */

	// informations serveur
	struct sockaddr_in localAddress;
	char* localAddressName = malloc(30*sizeof(char));

	// informations client
	struct sockaddr_in distantAddress;
	char* distantAddressName = malloc(30*sizeof(char));

	// informations sur la socket
	int socketId = 0;		// id de la socket
	socklen_t tailleSocket;	// taille de la socket

	// informations sur le message
	struct message sent;
	sent.seq = 0;
	sent.ack = 0;

	struct message received;
	received.seq = NOPE;
	received.ack = NOPE;

	// informations sur les paquets
	int attendu = 0;	// #DA
	int prochain = 0;	// #PDAE

	// fichiers
	int fichierRecu;
	int fichierEnvoye;

	// gestion de la terminaison
	int transmis = 0;	// message à envoyer transmis
	int recu = 0;		// message à recevoir reçu
	int accuse = 0;		// accusé de réception
	int fini = 0;		// quand les 2 autres sont vrais -> fini

	// générales
	int nbTransmis = 0;		// nombre de c transmis dans un message
	int nbTransmisTotal = 0;// nombre de c transmis en tout
	int nbRecu = 0;			// nombre de caractères reçus dans un message
	int nbRecuTotal = 0;	// nombre total de caractères reçus
	int nbLu = 0;			// nombre lu
	int etat = 0;			// retour du select

	/* Création de la socket UDP */
	if ((socketId = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
		fprintf(stderr, "Erreur lors de l'ouverture de la socket.\n");
		perror("socket");
		exit(2);
	}

	/* Création de la structure d'écoute */
	localAddress.sin_family = AF_INET;
	localAddress.sin_addr.s_addr = htonl(INADDR_ANY);

	if (argc == 6)
		localAddress.sin_port = htons(atoi(argv[5]));
	else
		localAddress.sin_port = htons(20000);

	/* Création de la structure du serveur */
	distantAddress.sin_family = AF_INET;

	// convertir a.b.c.d 
	if ((inet_pton(AF_INET, argv[3], &distantAddress.sin_addr)) != 1)
	{
		perror("inet_pton");
		exit(666);
	}

	distantAddress.sin_port = htons(atoi(argv[4]));

	tailleSocket = sizeof(localAddress);

	/* Binding de la socket d'écoute */	
	if ((bind(socketId, 
		(struct sockaddr*)&localAddress, 
		tailleSocket)) == -1)
	{
		fprintf(stderr, "Erreur lors du binding de la socket.\n");
		perror("bind");
		exit(3);
	}

	/* Ouverture des fichiers */
	if ((fichierEnvoye = open(argv[1], O_RDONLY)) < 0)
	{
		perror("open");
		exit(1);
	}
	if ((fichierRecu = open(argv[2], O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR)) < 0)
	{
		perror("open");
		exit(1);
	}

	/* Connexion */
	clientHandshake(socketId, &distantAddress);

	/*
		Remplissage du premier paquet.
		explications détaillées dans le while,
		section if (received.ack == prochain && !transmis)
	*/
	printf("\nOn écrit la première partie "
		"du fichier dans le buffer.\n");
	nbLu = read(fichierEnvoye, sent.buffer, BUFFER_LENGTH);

	sent.seq = prochain++;

	if (((unsigned long)nbLu < BUFFER_LENGTH))
	{
		transmis = 1;
		printf("\ntransmission terminée : %d < %lu\n\n", nbLu, sizeof(struct message));

	}

	/*
		Mise en attente bloquante jusqu'à ce que l'on ait reçu
		et envoyé	
	*/ printf("\n");
	while (!fini)
	{

		if(transmis && recu && accuse)
			fini = 1;
		/*
			On envoie un message à chaque fois
		*/
		if ((nbTransmis = sendto(socketId, 
						&sent, 
						nbLu + (sizeof(struct message) - BUFFER_LENGTH),
						0, 
						(struct sockaddr*)&distantAddress, 
						tailleSocket)) == -1)
		{
			printf("nbRecu : %d\n", nbRecu);
			printf("moins : %lu\n", (sizeof(struct message) - BUFFER_LENGTH));
			printf("nbLu - (sizeof(struct message) - BUFFER_LENGTH) : %lu", nbLu + (sizeof(struct message) - BUFFER_LENGTH));
			perror("sendto");
			exit(1);
		}
		printf("Envoyé %d char, seq = %d, "
			"ack = %d\n",
			nbTransmis, sent.seq, sent.ack);

		/*
			La méthode attente fait appel à select()
			afin de savoir si un message peut être
			reçu. Si select timeout alors on va juste
			renvoyer le message.
		*/
		etat = attente(socketId, TV_SEC, TV_USEC);
		
		if (etat != 0)	// réception
		{
			nbRecu = recvfrom(socketId, 
							&received, 
							sizeof(struct message), 
							0, 
							NULL, 
							&tailleSocket);

			printf("Reçu %d char dont %lu utiles, "
					"seq = %d, ack = %d\n", 
					nbRecu, 
					nbRecu - (sizeof(struct message) - BUFFER_LENGTH),
					received.seq, 
					received.ack);

			/*
				Si jamais le numéro de séquence
				est égal à celui que l'on attend
				alors on va :
				- écrire son contenu
				- mettre le bon numéro dans l'ack
				- mettre à jour le numéro attendu
			*/
			if (received.seq == attendu && !recu)
			{
				printf("Bon seq : sent.ack = ++%d\n", attendu);

				// on écrit nbRecu - l'en-tête
				printf("Ecriture.\n");
				write(fichierRecu, received.buffer, nbRecu - (sizeof(struct message) - BUFFER_LENGTH));

				// on met à jour l'ack et attendu + 1
				sent.ack = ++attendu;

				//printf("Envoi d'un ack : %d\n\n", attendu - 1);

				// si le buffer n'est pas plein -> on a fini de recevoir
				if ((unsigned long)nbRecu < sizeof(struct message))
				{
					recu = 1;
					printf("\nRéception terminée : %d < %lu\n\n", nbRecu, sizeof(struct message));
				}
			}

			/*
				Si jamais l'acquittement est égal au
				numéro du prochain datagramme que
				l'on attend alors :
				- on lit la suite de ce qu'on veut envoyer
				- on positionne seq au numéro du paquet
				- on incrémente le numéro du paquet
			*/
			if (received.ack == prochain && !transmis)
			{
				printf("Bon ack : sent.seq = %d++\n", prochain);
				printf("Lecture.\n");
				nbLu = read(fichierEnvoye, sent.buffer, BUFFER_LENGTH);

				/*
					On envoie la suite et on augmente
					"prochain" pour spécifier quel paquet
					on attend
				*/
				sent.seq = prochain++;

				//printf("Envoi d'un seq : %d\n\n", prochain);

				// on a moins lu que la taille du buffer
				// -> fin du fichier, et fin de la transmission
				if (((unsigned long)nbLu == 0))
				{
					transmis = 1;
					printf("\ntransmission terminée : %d < %lu\n\n", nbLu, sizeof(struct message));

				}
			}

			/*
				condition de fin :
				on regarde accusé seulement si les deux autres sont finis
			*/
			if(((unsigned long)nbRecu == (sizeof(struct message) - BUFFER_LENGTH)) && transmis && recu)
			{
				accuse = 1;
				printf("\nAccuse de reception recu.\n");
			}

			nbRecuTotal += nbRecu;
		}
		else
			printf("Timeout ! \n");

		nbTransmisTotal += nbTransmis;
	}

	/* Affichage des informations */
	printf("\nInformations : \n");

	inet_ntop(AF_INET, &localAddress.sin_addr.s_addr, localAddressName, 30);
	printf("Adresse IP locale  : %s \n", localAddressName);
	printf("port local : %d \n", ntohs(localAddress.sin_port));

	inet_ntop(AF_INET, &distantAddress.sin_addr.s_addr, distantAddressName, 30);
	printf("Adresse IP distante : %s \n", distantAddressName);
	printf("port distant : %d \n", ntohs(distantAddress.sin_port));

	printf("Nombre total de caractères transmis : %d \n", nbTransmisTotal);
	printf("Nombre total de caractères reçus : %d \n", nbRecuTotal);

	/* Phase de fermeture */
	free(distantAddressName);
	free(localAddressName);
	close(fichierEnvoye);
	close(fichierRecu);

	printf("\nFin du programme.\n");

	return 0;
}

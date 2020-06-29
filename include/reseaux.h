/*
	Par Andréas Guillot
	et Ervin Altintas
*/

#ifndef __RESEAUX_H
#define __RESEAUX_H

#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <time.h>

#define BUFFER_LENGTH 1016

#define TV_SEC 1
#define TV_USEC 50000
#define TV_MAX 10
	
#define NOPE -10

struct message
{
	int seq;
	int ack;
	char buffer[BUFFER_LENGTH];
};

void clientHandshake(int, struct sockaddr_in *);
void serverHandshake(int, struct sockaddr_in *);
int attente(int, int, int);

#endif

// da = attendu
// pdae = prochain
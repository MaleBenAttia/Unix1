#ifndef SERV_CLI_FIFO_H
#define SERV_CLI_FIFO_H

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define NMAX 100
#define FIFO1 "fifo1"
#define FIFO2 "fifo2"

/* Structure pour une question */
typedef struct {
    pid_t pid_client;
    int nombre;
} Question;

/* Structure pour une réponse */
typedef struct {
    pid_t pid_client;
    int taille;
    int valeurs[NMAX];
} Reponse;

/* Prototypes communs */
void generer_nombres_aleatoires(Reponse *rep, int n);
void fin_serveur();

#endif

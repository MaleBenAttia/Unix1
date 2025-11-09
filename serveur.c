#include "serv_cli_fifo.h"
#include "Handlers_Serv.h"

/* Handler pour SIGUSR1 */
void hand_reveil(int sig) {
    printf("Serveur: Signal SIGUSR1 reçu (réveil du serveur)\n");
}

/* Handler immortel */
void hand_immortel(int sig) {
    printf("\nServeur: Signal %d reçu mais ignoré! Le serveur est immortel!\n", sig);
    printf("Serveur: Pour arrêter le serveur, utilisez: kill -9 %d\n", getpid());
}

/* Nettoyage */
void fin_serveur() {
    printf("Serveur: Fin du serveur - Nettoyage et fermeture\n");
    unlink(FIFO1);
    unlink(FIFO2);
}

/* Génération des nombres */
void generer_nombres_aleatoires(Reponse *rep, int n) {
    int i;
    rep->taille = n;

    printf("Serveur: Génération de %d nombres aléatoires: ", n);
    for(i = 0; i < n; i++) {
        rep->valeurs[i] = rand() % (NMAX + 1);
        printf("%d ", rep->valeurs[i]);
    }
    printf("\n");
}

int main() {
    Question quest;
    Reponse rep;
    int fd_fifo1, fd_fifo2;

    printf("Serveur: Démarrage du serveur (PID: %d)\n", getpid());

    /* Installation des handlers */
    signal(SIGUSR1, hand_reveil);
    signal(SIGINT,  hand_immortel);
    signal(SIGTERM, hand_immortel);
    signal(SIGQUIT, hand_immortel);
    signal(SIGTSTP, hand_immortel);

    printf("Serveur: ⚡ Mode IMMORTEL activé!\n");

    srand(getpid());

    /* Création des FIFOs */
    mkfifo(FIFO1, 0666);
    mkfifo(FIFO2, 0666);

    printf("Serveur: Tubes nommés créés (fifo1, fifo2)\n");
    printf("Server: En attente de questions...\n");

    while(1) {
        fd_fifo1 = open(FIFO1, O_RDONLY);

        if(read(fd_fifo1, &quest, sizeof(Question)) > 0) {
            printf("Serveur: Question reçue du client (PID: %d) - Nombre demandé: %d\n",
                   quest.pid_client, quest.nombre);

            if(quest.nombre > 0 && quest.nombre <= NMAX) {
                rep.pid_client = quest.pid_client;
                generer_nombres_aleatoires(&rep, quest.nombre);

                printf("Serveur: Envoi du signal SIGUSR1 au client\n");
                kill(quest.pid_client, SIGUSR1);

                usleep(100000);

                fd_fifo2 = open(FIFO2, O_WRONLY);
                write(fd_fifo2, &rep, sizeof(Reponse));
                close(fd_fifo2);

                printf("Serveur: Réponse envoyée\n");
            }
        }

        close(fd_fifo1);
    }

    fin_serveur();
    return 0;
}

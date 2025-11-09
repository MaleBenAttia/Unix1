#include "serv_cli_fifo.h"
#include "Handlers_Cli.h"

int reponse_prete = 0;

/* Handler */
void hand_reveil(int sig) {
    printf("\nClient: Signal SIGUSR1 reçu - Réponse prête\n");
    reponse_prete = 1;
}

int main() {
    Question quest;
    Reponse rep;
    int fd_fifo1, fd_fifo2;
    int nombre_demande, choix, i;
    int continuer = 1;

    printf("Client: Démarrage du client (PID: %d)\n", getpid());

    signal(SIGUSR1, hand_reveil);

    while(continuer) {
        reponse_prete = 0;

        printf("\nClient: Combien de nombres aléatoires voulez-vous (1-%d) ? ", NMAX);
        scanf("%d", &nombre_demande);

        if(nombre_demande < 1 || nombre_demande > NMAX) {
            printf("Client: Nombre invalide\n");
            continue;
        }

        quest.pid_client = getpid();
        quest.nombre = nombre_demande;

        printf("Client: Envoi de la question...\n");
        fd_fifo1 = open(FIFO1, O_WRONLY);
        write(fd_fifo1, &quest, sizeof(Question));
        close(fd_fifo1);

        printf("Client: En attente du serveur...\n");
        while(!reponse_prete) pause();

        fd_fifo2 = open(FIFO2, O_RDONLY);

        if(read(fd_fifo2, &rep, sizeof(Reponse)) > 0) {
            printf("Client: Réponse reçue:\n");
            for(i = 0; i < rep.taille; i++)
                printf("%d ", rep.valeurs[i]);
            printf("\n");
        }

        close(fd_fifo2);

        printf("\nClient: 1 - Quitter, autre - continuer\n");
        scanf("%d", &choix);
        if(choix == 1) continuer = 0;
    }

    return 0;
}

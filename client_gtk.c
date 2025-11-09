// client_gtk.c - Version commentée
// Compilation : gcc client_gtk.c -o client_gtk `pkg-config --cflags --libs gtk+-3.0` -pthread
//sudo apt update
//sudo apt install libgtk-3-dev


#include <gtk/gtk.h>          // Bibliothèque GTK3
#include <pthread.h>          // Threads POSIX
#include <signal.h>           // Gestion des signaux UNIX
#include <unistd.h>           // PID, pause, open, read, write
#include <fcntl.h>            // open()
#include <stdio.h>            // printf
#include <stdlib.h>           // atoi
#include <string.h>           // strcat

#include "serv_cli_fifo.h"   // Structures Question et Reponse
#include "Handlers_Cli.h"    // Handler SIGUSR1

/* Widgets GTK globaux */
GtkWidget *entry_nombre;        // Champ de saisie du nombre
GtkWidget *textview_reponse;    // Zone d'affichage des valeurs

/* Variables de synchronisation */
int reponse_prete = 0;          // Indique que SIGUSR1 a été reçu
pthread_t thread_signal;        // Thread d'attente des signaux

/* Handler appelé quand SIGUSR1 arrive */
void hand_reveil(int sig) {
    reponse_prete = 1;          // Débloque l'attente de la réponse
}

/* Thread bloqué sur sigwait(), pour ne pas bloquer GTK */
void *attente_signal(void *arg) {
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);   // On attend uniquement SIGUSR1

    while (1) {
        int sig;
        sigwait(&set, &sig);    // Bloque jusqu'à réception signal
        reponse_prete = 1;      // Débloque la lecture FIFO2
    }
    return NULL;
}

/* Fonction appelée lors du clic sur le bouton "Envoyer" */
void envoyer_demande(GtkWidget *widget, gpointer data) {
    // Récupère le texte dans l'entrée GTK
    const char *txt = gtk_entry_get_text(GTK_ENTRY(entry_nombre));
    int nombre = atoi(txt);

    // Vérification de validité
    if (nombre < 1 || nombre > NMAX) return;

    // Construction de la structure Question
    Question q;
    q.pid_client = getpid();
    q.nombre = nombre;

    // Ouverture du FIFO1 en écriture → envoi de la Question
    int fd = open(FIFO1, O_WRONLY);
    write(fd, &q, sizeof(Question));
    close(fd);

    // Attente du signal SIGUSR1 (réponse prête)
    while(!reponse_prete)
        usleep(10000);   // Petite attente non bloquante

    reponse_prete = 0;   // Réinitialisation

    // Lecture de la réponse depuis FIFO2
    Reponse rep;
    fd = open(FIFO2, O_RDONLY);
    read(fd, &rep, sizeof(Reponse));
    close(fd);

    // Construction du texte affiché
    char buffer[2048] = "";
    for(int i = 0; i < rep.taille; i++) {
        char tmp[32];
        sprintf(tmp, "%d ", rep.valeurs[i]);
        strcat(buffer, tmp);
    }

    // Affichage dans la zone texte GTK
    GtkTextBuffer *buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview_reponse));
    gtk_text_buffer_set_text(buf, buffer, -1);
}

/* Programme principal : construction de l'interface GTK */
int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);     // Initialisation GTK

    /* Blocage du signal SIGUSR1 dans le thread principal */
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    pthread_sigmask(SIG_BLOCK, &set, NULL);

    // Création du thread chargé d'attendre les signaux
    pthread_create(&thread_signal, NULL, attente_signal, NULL);

    // Installation du handler (ne sera pas exécuté directement car sigwait intercepte)
    signal(SIGUSR1, hand_reveil);

    /* --- Création de la fenêtre GTK --- */
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Client GTK FIFO");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);

    GtkWidget *label = gtk_label_new("Nombre demandé :");
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 5);

    entry_nombre = gtk_entry_new();                 // Champ de saisie
    gtk_box_pack_start(GTK_BOX(hbox), entry_nombre, TRUE, TRUE, 5);

    GtkWidget *btn = gtk_button_new_with_label("Envoyer");
    gtk_box_pack_start(GTK_BOX(hbox), btn, FALSE, FALSE, 5);

    // Connexion du bouton à la fonction d'envoi
    g_signal_connect(btn, "clicked", G_CALLBACK(envoyer_demande), NULL);

    // Cadre contenant la zone de réponse
    GtkWidget *frame = gtk_frame_new("Réponse du serveur :");
    gtk_box_pack_start(GTK_BOX(vbox), frame, TRUE, TRUE, 5);

    // Zone d'affichage
    textview_reponse = gtk_text_view_new();
    gtk_container_add(GTK_CONTAINER(frame), textview_reponse);

    // Quitter proprement
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(window);
    gtk_main();     // Boucle GTK

    return 0;
}

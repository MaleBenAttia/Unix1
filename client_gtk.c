// gcc client_gtk.c -o client_gtk `pkg-config --cflags --libs gtk+-3.0` -pthread

#include <gtk/gtk.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NMAX 100
#define FIFO1 "fifo1"
#define FIFO2 "fifo2"

/* Structures originales */
typedef struct {
    pid_t pid_client;
    int nombre;
} Question;

typedef struct {
    pid_t pid_client;
    int taille;
    int valeurs[NMAX];
} Reponse;

/* Variables globales */
GtkWidget *entry_nombre;
GtkWidget *textview_reponse;
int reponse_prete = 0;
pthread_t thread_signal;

/* Handler signal */
void hand_reveil(int sig) {
    reponse_prete = 1;
}

/* Thread d'attente du signal */
void *attente_signal(void *arg) {
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);

    while (1) {
        int sig;
        sigwait(&set, &sig);
        reponse_prete = 1;
    }
    return NULL;
}

/* Fonction pour envoyer une demande */
void envoyer_demande(GtkWidget *widget, gpointer data) {
    const char *txt = gtk_entry_get_text(GTK_ENTRY(entry_nombre));
    int nombre = atoi(txt);
    if (nombre < 1 || nombre > NMAX) return;

    Question q;
    q.pid_client = getpid();
    q.nombre = nombre;

    int fd = open(FIFO1, O_WRONLY);
    write(fd, &q, sizeof(Question));
    close(fd);

    while(!reponse_prete) usleep(10000);
    reponse_prete = 0;

    Reponse rep;
    fd = open(FIFO2, O_RDONLY);
    read(fd, &rep, sizeof(Reponse));
    close(fd);

    char buffer[2048] = "";
    for(int i=0;i<rep.taille;i++){
        char tmp[32];
        sprintf(tmp, "%d ", rep.valeurs[i]);
        strcat(buffer, tmp);
    }

    GtkTextBuffer *buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview_reponse));
    gtk_text_buffer_set_text(buf, buffer, -1);
}

/* Interface */
int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    /* Thread pour signal */
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    pthread_sigmask(SIG_BLOCK, &set, NULL);
    pthread_create(&thread_signal, NULL, attente_signal, NULL);

    signal(SIGUSR1, hand_reveil);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Client GTK FIFO");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);

    GtkWidget *label = gtk_label_new("Nombre demandé :");
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 5);

    entry_nombre = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(hbox), entry_nombre, TRUE, TRUE, 5);

    GtkWidget *btn = gtk_button_new_with_label("Envoyer");
    gtk_box_pack_start(GTK_BOX(hbox), btn, FALSE, FALSE, 5);
    g_signal_connect(btn, "clicked", G_CALLBACK(envoyer_demande), NULL);

    GtkWidget *frame = gtk_frame_new("Réponse du serveur :");
    gtk_box_pack_start(GTK_BOX(vbox), frame, TRUE, TRUE, 5);

    textview_reponse = gtk_text_view_new();
    gtk_container_add(GTK_CONTAINER(frame), textview_reponse);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(window);
    gtk_main();
    return 0;
}

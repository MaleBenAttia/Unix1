# Compilateur et options
CC = gcc
CFLAGS = -Wall

# Options GTK3
GTKFLAGS = `pkg-config --cflags --libs gtk+-3.0` -pthread

# Cibles principales
all: serveur client client_gtk

# Compilation du serveur
serveur: serveur.c
	$(CC) $(CFLAGS) -o serveur serveur.c

# Compilation du client console
client: client.c
	$(CC) $(CFLAGS) -o client client.c

# Compilation du client GTK
client_gtk: client_gtk.c
	$(CC) $(CFLAGS) -o client_gtk client_gtk.c $(GTKFLAGS)

# Exécution du client GTK
run_gtk: client_gtk
	./client_gtk

# Nettoyage des fichiers compilés et tubes
clean:
	rm -f serveur client client_gtk fifo1 fifo2

# Nettoyage complet
cleanall: clean
	rm -f *~

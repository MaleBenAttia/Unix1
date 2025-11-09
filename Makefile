# Compilateur et options
CC = gcc
CFLAGS = -Wall

# Cibles principales
all: serveur client

# Compilation du serveur
serveur: serveur.c
	$(CC) $(CFLAGS) -o serveur serveur.c

# Compilation du client
client: client.c
	$(CC) $(CFLAGS) -o client client.c

# Nettoyage des fichiers compilés et tubes
clean:
	rm -f serveur client fifo1 fifo2

# Nettoyage complet
cleanall: clean
	rm -f *~

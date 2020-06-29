DEBUG = YES
CC = gcc

CFLAGS = -W
LFLAGS = -lpcap -lm

DIRECTORIES = src include obj bin

# Debug permets d'utiliser gdb
ifeq ($(DEBUG), YES)
		override CFLAGS := -g $(CFLAGS)
endif

# Les wildcards indiquent où sont les fichiers
CLIENT_HEADERS = include/reseaux.h
SERVER_HEADERS = include/reseaux.h
MEDIUM_HEADERS = include/reseaux.h include.socket.h include/fifo.h include/timer.h

CLIENT_SOURCES = src/client.c src/reseaux.c
CLIENT_OBJECTS = $(CLIENT_SOURCES:src/%.c=obj/%.o)

SERVER_SOURCES = src/serveur.c src/reseaux.c
SERVER_OBJECTS = $(SERVER_SOURCES:src/%.c=obj/%.o)

MEDIUM_SOURCES = src/medium.c src/socket.c src/fifo.c src/timer.c src/reseaux.c
MEDIUM_OBJECTS = $(MEDIUM_SOURCES:src/%.c=obj/%.o)

# les .PHONY représentent toutes les règles qu'il ne faut pas confondre avec des fichiers.
# -> si jamais un fichier "clean" existe, make l'ignorera si jamais on fait "make clean".
.PHONY : all clean dir archive

# $@ est le nom du fichier concerné par la règle
# $< le nom du premier prérequis

all : dir programs
ifeq ($(DEBUG), YES)
	@echo "==== Generated in debug mode ===="
else
	@echo "==== Generated in release mode ===="
endif

programs : bin/client bin/server bin/medium

bin/client : $(CLIENT_OBJECTS) $(HEADERS)
	@echo "\n==== Creating the client ===="
	$(CC) $(CFLAGS) -o $@ $(CLIENT_OBJECTS) $(LFLAGS)
	@echo ""

bin/server : $(SERVER_OBJECTS) $(HEADERS)
	@echo "\n==== Creating the server ===="
	$(CC) $(CFLAGS) -o $@ $(SERVER_OBJECTS) $(LFLAGS)
	@echo ""

bin/medium : $(MEDIUM_OBJECTS) $(HEADERS)
	@echo "\n==== Creating the medium ===="
	$(CC) $(CFLAGS) -o $@ $(MEDIUM_OBJECTS) $(LFLAGS)
	@echo ""

obj/%.o : src/%.c
	@echo "\n---- Rule " $@ "----"
	$(CC) $(CFLAGS) -c $<
	@mv *.o obj

test : all
	bin/medium 4 10000 127.0.0.1 5555 127.0.0.1 6666 0 0 &
	bin/server util/pika.jpeg bin/pika1.jpeg 5555 &
	bin/client util/pika.jpeg bin/pika2.jpeg 127.0.0.1 10000 6666
	@diff -s util/pika.jpeg bin/pika1.jpeg
	@diff -s util/pika.jpeg bin/pika2.jpeg

# clean supprime les .o
clean :
	@echo "==== Cleaning ===="
	rm obj/*.o bin/* 2>/dev/null

# create directories.
dir :
	@for dir in $(DIRECTORIES); do \
		exists=$$([ -d $$dir ]; echo $$?); \
		if [ "$$exists" -eq "1" ]; then \
				mkdir $$dir; \
		fi \
	done

archive :
	@echo "==== Archiving ===="
	zip andreas_guillot.zip -r src/ include/

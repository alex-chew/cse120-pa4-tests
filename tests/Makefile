# Makefile to compile Umix Programming Assignment 4 (pa4) [updated: 1/17/18]

LIBDIR = $(UMIXPUBDIR)/lib
# LIBDIR = $(UMIXROOTDIR)/sys

CC 	= cc
FLAGS 	= -g -L$(LIBDIR) -lumix4
SRC     =
OBJ     = $(SRC:.c=.o)

%.o: %.c
	$(CC) $(FLAGS) $(REFFLAG) -c $<

all: clean $(OBJ)

clean:
	rm -f *.o

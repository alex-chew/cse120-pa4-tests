# Makefile to compile Umix Programming Assignment 4 (pa4) [updated: 1/17/18]

LIBDIR = $(UMIXPUBDIR)/lib
# LIBDIR = $(UMIXROOTDIR)/sys

CC 	= cc 
FLAGS 	= -g -L$(LIBDIR) -lumix4

PA4 =	pa4a pa4b pa4c
TESTS = mytest reftest

pa4:	$(PA4)

tests: $(TESTS)

pa4a:	pa4a.c aux.h umix.h
	$(CC) $(FLAGS) -o pa4a pa4a.c

pa4b:	pa4b.c aux.h umix.h mykernel4.h mykernel4.o
	$(CC) $(FLAGS) -o pa4b pa4b.c mykernel4.o

pa4c:	pa4c.c aux.h umix.h mykernel4.h mykernel4.o
	$(CC) $(FLAGS) -o pa4c pa4c.c mykernel4.o

mykernel4.o:	mykernel4.c aux.h umix.h mykernel4.h
	$(CC) $(FLAGS) -c mykernel4.c

mytest: tests.c aux.h umix.h mykernel4.h mykernel4.o
	$(CC) $(FLAGS) -o $@ tests.c mykernel4.o

reftest: tests.c aux.h umix.h mykernel4.h mykernel4.o
	$(CC) $(FLAGS) -DUSE_REFERENCE_KERNEL -o $@ tests.c mykernel4.o

clean:
	rm -f *.o $(PA4) $(TESTS)

CC = gcc
CFLAGS = -Wall -std=c99 -pedantic -g 
MAIN = main
OBJS = main.o array_list.o 

all : $(MAIN) 

$(MAIN) : $(OBJS) main.h 
	$(CC) $(CFLAGS) -o $(MAIN) $(OBJS) 


tree.o : main.c main.h 
	$(CC) $(CFLAGS) -c tree.c

array_list.o : array_list.c array_list.h
	$(CC) $(CFLAGS) -c array_list.c

clean: 
	rm *.o $(MAIN) core 

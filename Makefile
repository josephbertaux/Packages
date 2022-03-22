PROG = program
CC = g++
CPPFLAGS = -c -g -Wall -fpic
LDFLAGS = 
ROOTFLAGS = -lTMVA -lTMVAGui `root-config --cflags --evelibs --auxlibs`
OBJS = main.o LumberJack.o

$(PROG) : $(OBJS)
	$(CC) $(LDFLAGS) -o $(PROG) $(OBJS) $(ROOTFLAGS)

main.o :
	$(CC) $(CPPFLAGS) main.cpp $(ROOTFLAGS)

LumberJack.o : LumberJack.h
	$(CC) $(CPPFLAGS) LumberJack.cpp $(ROOTFLAGS)

clean:
	rm -f core $(PROG) $(OBJS)


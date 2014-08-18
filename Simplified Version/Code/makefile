
# Specify the compiler
CC = g++

#CCOPTS = -ansi -pedantic -Wall -g
CCOPTS = -ggdb -Wall
LFLAG = -l
IFLAG = -I
LIBFLAG = -L
LIBDIR = /usr/local/lib/
INCLUDEDIR = /usr/local/include/
LIBS = -lpthread

# Make the source
all:	host router

common.o : common.h common.cpp 
	$(CC) $(CCOPTS) -c common.cpp

CommonProject.o : CommonProject.h CommonProject.cpp 
	$(CC) $(CCOPTS) -c CommonProject.cpp	
 
host:  host.cpp common.o CommonProject.o  
	$(CC) $(CCOPTS) common.o CommonProject.o host.cpp -o host $(LIBS)

router: router.cpp common.o CommonProject.o
	$(CC) $(CCOPTS) $(IFLAG) $(INCLUDEDIR) $(LIBFLAG)$(LIBDIR) $(LIBS) common.o CommonProject.o router.cpp -o router $(LFLAG)libboost_regex	-lboost_system  -lboost_thread-mt


clean :
	rm -f common.o CommonProject.o host router

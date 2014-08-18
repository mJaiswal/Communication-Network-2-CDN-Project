#Specify the compiler
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
all: Router Host

common.o : common.h common.cpp 
	$(CC) $(CCOPTS) -c common.cpp

CommonProject.o : common.h CommonProject.h CommonProject.cpp
	$(CC) $(CCOPTS) -c CommonProject.cpp

Router: common.o CommonProject.o   Router.h Router.cpp
	$(CC) $(CCOPTS) $(IFLAG) $(INCLUDEDIR) $(LIBFLAG)$(LIBDIR) $(LIBS)  common.o CommonProject.o  Router.cpp -o Router  $(LFLAG)libboost_regex	-lboost_system  -lboost_thread-mt                  

Host : common.o CommonProject.o host.cpp
	$(CC) $(CCOPTS)  common.o CommonProject.o  host.cpp -o Host $(LIBS)

clean :
	rm -f common.o CommonProject.o Router Host
CC       = g++
CFLAGS   = -Wall -g -O -fPIC     
CXXFLAGS = 
INCLUDE  = -I ../include
TARGET   = libzywco.so
LIBPATH  = ../libs/
SC_PATH  =../core
vpath %.h ./inc

EXEC     = main


all:
	cd $(SC_PATH) && make clean && make
	$(CC) -o $(EXEC) example_join.cpp -L$(LIBPATH) -lzywco  -ldl -D_GNU_SOURCE $(INCLUDE)  -Wl,--rpath=$(LIBPATH)  -std=c++11

	

clean:
	rm -f *.o $(EXEC)
	rm -f $(LIBPATH)*










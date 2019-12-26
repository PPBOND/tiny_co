CC       = g++
CFLAGS   = -Wall -g -O -fPIC     
CXXFLAGS = 
INCLUDE  = -I ../include
TARGET   = libzywco.so
LIBPATH  = ../libs/

vpath %.h ./inc

OBJS     = event.o     coroutine.o        timer.o   swapcontext.o
SRCS     = event.cpp   coroutine.cpp      timer.cpp swapcontext.S


all:$(OBJS)
	$(CC) -shared -fPIC -o $(TARGET) $(OBJS)    -std=c++11
	mv $(TARGET) $(LIBPATH)
$(OBJS):$(SRCS)
	$(CC) $(CFLAGS) $(INCLUDE) -c $^  -std=c++11
	


clean:
	rm -f *.o 
	rm -f $(LIBPATH)*










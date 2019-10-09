CC       = g++
CFLAGS   = -Wall -g -O -fPIC     
CXXFLAGS = 
INCLUDE  = -I ../include
TARGET   = libzywco.so
LIBPATH  = ./libs/

vpath %.h ./inc

OBJS     = event.o socket.o thread.o
SRCS     = event.cpp  socket.cpp thread.cpp 




all:$(OBJS)
	$(CC) -shared -fPIC -o $(TARGET) $(OBJS)    -std=c++11
	mv $(TARGET) $(LIBPATH)
	$(CC) -o main main.cpp -L$(LIBPATH) -lzywco  $(INCLUDE)  -Wl,--rpath=$(LIBPATH)  -std=c++11

$(OBJS):$(SRCS)
	$(CC) $(CFLAGS) $(INCLUDE) -c $^  -std=c++11
	


clean:
	rm -f *.o
	rm -f $(LIBPATH)*










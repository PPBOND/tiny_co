CC       = g++
CFLAGS   = -Wall -g -O -fPIC     
CXXFLAGS = 
INCLUDE  = -I ../include
TARGET   = libzywco.so
LIBPATH  = ./libs/

vpath %.h ./inc

OBJS     = event.o socket.o coroutine.o   hook_sys_call.o co_cond.o timer.o	swapcontext.o
SRCS     = event.cpp  socket.cpp coroutine.cpp  hook_sys_call.cpp co_cond.cpp timer.cpp swapcontext.S
EXEC     = main


all:$(OBJS)
	$(CC) -shared -fPIC -o $(TARGET) $(OBJS)    -std=c++11
	mv $(TARGET) $(LIBPATH)
	$(CC) -o $(EXEC) main.cpp -L$(LIBPATH) -lzywco  -ldl -D_GNU_SOURCE $(INCLUDE)  -Wl,--rpath=$(LIBPATH)  -std=c++11

$(OBJS):$(SRCS)
	$(CC) $(CFLAGS) $(INCLUDE) -c $^  -std=c++11
	


clean:
	rm -f *.o $(EXEC)
	rm -f $(LIBPATH)*










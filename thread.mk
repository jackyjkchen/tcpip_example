CFLAGS = -O2 -Wall -pipe -pthread
CXXFLAGS = ${CFLAGS}
LDFLAGS =

progs = client_thread.exe

all: ${progs}

queue.o: deque.o deque.h

thread_pool_c.o: thread_pool_c.c thread_pool_c.h

client_thread.exe: client_thread.o io_client.o io_common.o thread_pool_c.o deque.o
	${CC} ${CFLAGS} client_thread.o io_client.o io_common.o thread_pool_c.o deque.o ${LDFLAGS} -o $@

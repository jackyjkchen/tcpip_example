CFLAGS = -O2 -Wall -pipe -pthread
CXXFLAGS = ${CFLAGS}
LDFLAGS = -Wl,--as-needed

progs = client_thread

all: ${progs}

thread_pool.o: thread_pool.cpp thread_pool.h
	${CXX} ${CXXFLAGS} -c -o $@ thread_pool.cpp

thread_pool_c.o: thread_pool_c.cpp thread_pool_c.h
	${CXX} ${CXXFLAGS} -c -o $@ thread_pool_c.cpp

client_thread: client_thread.o io_client.o io_common.o thread_pool_c.o thread_pool.o
	${CXX} ${CXXFLAGS} client_thread.o io_client.o io_common.o thread_pool_c.o thread_pool.o ${LDFLAGS} -o $@.exe

#ifndef _IO_COMMON_H_
#define _IO_COMMON_H_

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#ifndef SHUT_WR
#define SHUT_WR SD_SEND
#endif
#else
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
typedef int SOCKET;
#endif

#define SERV_PORT 9876
#define BUF_SIZE  4096

#ifdef __cplusplus
extern "C" {
#endif

extern ssize_t recvn(SOCKET fd, void *buf, size_t buf_size);

extern ssize_t sendn(SOCKET fd, const void *buf, size_t buf_size);

#ifndef _WIN32
extern int closesocket(int fd);
extern int set_rlimit();
#endif

#ifdef __cplusplus
}
#endif

#endif


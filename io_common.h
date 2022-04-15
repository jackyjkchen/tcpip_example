#ifndef _IO_COMMON_H_
#define _IO_COMMON_H_

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <winsock2.h>
typedef int socklen_t;
#ifndef _POSIX_C_SOURCE
typedef int ssize_t;
#endif
#define IO_SHUT_RD SD_RECEIVE
#define IO_SHUT_WR SD_SEND
#define IO_EWOULDBLOCK WSAEWOULDBLOCK
#define IO_EINTR WSAEINTR
#define MSG_NOSIGNAL 0
#else
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
typedef int SOCKET;
#define IO_SHUT_RD SHUT_RD
#define IO_SHUT_WR SHUT_WR
#define IO_EWOULDBLOCK EWOULDBLOCK
#define IO_EINTR EINTR
#endif
#define IO_OK 0

#define TCP 0
#define UDP 1

#define SERV_PORT 9876
#define BUF_SIZE  4096

#ifdef __cplusplus
extern "C" {
#endif

extern int close_socket(SOCKET fd);

extern int get_last_error();

extern void print_error(const char *msg);

#ifndef _WIN32
extern int set_rlimit();
#endif

#ifdef __cplusplus
}
#endif

#endif


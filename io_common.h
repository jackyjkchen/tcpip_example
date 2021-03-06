#ifndef _IO_COMMON_H_
#define _IO_COMMON_H_

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32

#include <winsock2.h>
typedef int socklen_t;
#ifndef __MINGW32__
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
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
typedef int SOCKET;
#define INVALID_SOCKET -1

/* for old libc */
#if __GNU_LIBRARY__ == 1 || defined(OLD_BSD)
typedef int socklen_t;
#ifndef SHUT_RD
#define SHUT_RD 0
#endif
#ifndef SHUT_WR
#define SHUT_WR 1
#endif
#endif /*USE_OLD_LIBC*/

#ifndef MSG_NOSIGNAL
#if __minix__
#define MSG_NOSIGNAL 0
#else
#define MSG_NOSIGNAL 0x4000
#endif
#endif
#define IO_SHUT_RD SHUT_RD
#define IO_SHUT_WR SHUT_WR
#define IO_EWOULDBLOCK EWOULDBLOCK
#define IO_EINTR EINTR

#endif

#define IO_OK 0
#define TCP 0
#define UDP 1

#define TCP_SERV_PORT 9876
#define UDP_SERV_PORT 6789
#define TCP_DATA_SIZE 1024*1024
#define TCP_BUF_SIZE  256*1024
#define UDP_BUF_SIZE  548

#ifdef __cplusplus
extern "C" {
#endif

extern int close_socket(SOCKET fd);

extern int get_last_error();

extern void set_last_error(int err);

extern void print_error(const char *msg);

#ifndef _WIN32
extern int set_rlimit();
#endif

#ifdef __cplusplus
}
#endif

#endif


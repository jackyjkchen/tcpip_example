#ifndef _IO_COMMON_H_
#define _IO_COMMON_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERV_PORT 9876
#define LISTENQ   1024
#define BUF_SIZE  4096

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_CONN  10240

extern int server_socket_init(int nonblock);

extern ssize_t readn(int fd, void *buf, size_t buf_size);

extern ssize_t writen(int fd, const void *buf, size_t buf_size);

extern int set_rlimit();

typedef int (*server_callback)(void *param);

extern int reflect_server_callback(void *param);

extern void reflect_client_callback(void *param);

extern void poll_loop(int listenfd, server_callback svrcbk);

extern void select_loop(int listenfd, server_callback svrcbk);

extern void accept_loop(int listenfd, server_callback svrcbk);

#ifdef __cplusplus
}
#endif

#endif


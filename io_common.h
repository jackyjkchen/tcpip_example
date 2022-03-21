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
#define BUF_SIZE  4096

#ifdef __cplusplus
extern "C" {
#endif

extern ssize_t readn(int fd, void *buf, size_t buf_size);

extern ssize_t writen(int fd, const void *buf, size_t buf_size);

extern int set_rlimit();

#ifdef __cplusplus
}
#endif

#endif


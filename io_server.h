#ifndef _IO_SERVER_H_
#define _IO_SERVER_H_

#include "io_common.h"
#include "io_context.h"

#define LISTENQ   1024

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_CONN  10240

typedef void (*server_callback)(void *param);

extern int server_socket_init(int protocol, int nonblock);

extern void reflect_server_callback(void *param);

#ifdef __cplusplus
}
#endif

#endif


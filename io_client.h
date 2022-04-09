#ifndef _IO_CLIENT_H_
#define _IO_CLIENT_H_

#include <stdlib.h>
#include "io_common.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void reflect_client_callback(void *param);

extern int client_socket_init(const char *straddr, struct sockaddr_in *pserver_addr);

#ifdef __cplusplus
}
#endif

#endif


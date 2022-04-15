#ifndef _WSAPOLL_COMMON_H_
#define _WSAPOLL_COMMON_H_

#include "io_server.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void WSAPoll_loop(SOCKET listenfd, server_callback svrcbk);

#ifdef __cplusplus
}
#endif

#endif

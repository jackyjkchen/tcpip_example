#ifndef _POLL_COMMON_H_
#define _POLL_COMMON_H_

#include "io_server.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void poll_loop(SOCKET listenfd, server_callback svrcbk);

#ifdef __cplusplus
}
#endif

#endif

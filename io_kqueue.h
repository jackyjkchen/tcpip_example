#ifndef _KQUEUE_COMMON_H_
#define _KQUEUE_COMMON_H_

#include "io_server.h"

#ifdef __cplusplus
extern "C" {
#endif

extern int kqueue_loop(SOCKET listenfd, server_callback svrcbk);

#ifdef __cplusplus
}
#endif

#endif

#ifndef _SELECT_COMMON_H_
#define _SELECT_COMMON_H_

#include "io_server.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void select_loop(SOCKET listenfd, server_callback svrcbk);

#ifdef __cplusplus
}
#endif

#endif

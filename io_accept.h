#ifndef _ACCEPT_COMMON_H_
#define _ACCEPT_COMMON_H_

#include "io_server.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void accept_loop(int listenfd, server_callback svrcbk);

#ifdef __cplusplus
}
#endif

#endif

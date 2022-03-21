#ifndef _EPOLL_COMMON_H_
#define _EPOLL_COMMON_H_

#include "io_common.h"

#ifdef __cplusplus
extern "C" {
#endif

extern int epoll_loop(int listenfd, server_callback svrcbk);

#ifdef __cplusplus
}
#endif

#endif

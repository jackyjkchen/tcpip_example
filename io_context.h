#ifndef _io_context_t_H_
#define _io_context_t_H_

#include <stdlib.h>
#include "io_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct io_context_t {
    void *fd;
    char *buf;
    ssize_t bufsize;
    ssize_t recvbytes;
    ssize_t sendbytes;
    int sendagain;
    int recvdone;
} io_context_t;

extern void alloc_io_context(void *key);

extern void free_io_context(void *key);

extern io_context_t *get_io_context(void *key);

#ifdef __cplusplus
}
#endif

#endif


#include "io_select.h"

#ifdef __cplusplus
extern "C" {
#endif

int accept_loop(SOCKET listenfd, server_callback svrcbk)
{
    io_context_t io_context;
    io_context.fd = NULL;
    io_context.buf = NULL;
    io_context.bufsize = BUF_SIZE;
    io_context.recvbytes = 0;
    io_context.sendbytes = 0;
    io_context.buf = malloc(io_context.bufsize);
    if (io_context.buf == NULL) {
        print_error("alloc_io_context failed");
        return -1;
    }
    for (;;) {
        SOCKET connfd = accept(listenfd, NULL, NULL);
#ifdef _WIN32
        io_context.fd = (void *)connfd;
#else
        io_context.fd = (void *)(long)connfd;
#endif
        if (connfd < 0) {
            print_error("Accept failed");
            continue;
        }
        svrcbk(&io_context);
    }
    return 0;
}

#ifdef __cplusplus
}
#endif


#include "io_select.h"

#ifdef __cplusplus
extern "C" {
#endif

void accept_loop(SOCKET listenfd, server_callback svrcbk)
{
    SOCKET connfd;

    for (;;) {
        connfd = accept(listenfd, NULL, NULL);
        if (connfd < 0) {
            perror("Accept failed");
            continue;
        }
#ifdef _WIN32
        svrcbk((void*)connfd);
#else
        svrcbk((void*)(long)connfd);
#endif
    }
}

#ifdef __cplusplus
}
#endif


#include "io_select.h"

#ifdef __cplusplus
extern "C" {
#endif

void accept_loop(SOCKET listenfd, server_callback svrcbk)
{
    SOCKET connfd;
    socklen_t client_addr_len;
    struct sockaddr_in client_addr;

    for (;;) {
        client_addr_len = sizeof(client_addr);
        connfd = accept(listenfd, (struct sockaddr *)&client_addr, &client_addr_len);
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


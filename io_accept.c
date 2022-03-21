#include "io_select.h"

#ifdef __cplusplus
extern "C" {
#endif

void accept_loop(int listenfd, server_callback svrcbk)
{
    int connfd;
    socklen_t client_addr_len;
    struct sockaddr_in client_addr;

    for (;;) {
        client_addr_len = sizeof(client_addr);
        connfd = accept(listenfd, (struct sockaddr *)&client_addr, &client_addr_len);
        if (connfd < 0) {
            perror("Accept failed");
            continue;
        }
        svrcbk((void*)(long)connfd);
    }
}

#ifdef __cplusplus
}
#endif


#include <poll.h>
#include "io_poll.h"

#ifdef __cplusplus
extern "C" {
#endif

void poll_loop(SOCKET listenfd, server_callback svrcbk)
{
    SOCKET connfd;
    int i = 0, maxi = 0;
    socklen_t client_addr_len;
    struct sockaddr_in client_addr;
    struct pollfd client[MAX_CONN];

    client[0].fd = listenfd;
    client[0].events = POLLRDNORM;
    for (i=1; i<MAX_CONN; ++i) {
        client[i].fd = -1;
    }

    for (;;) {
        int ready_num = poll(client, maxi + 1, -1);
        if (ready_num < 0) {
            perror("Poll failed");
        }

        if (client[0].revents == POLLRDNORM) {
            client_addr_len = sizeof(client_addr);
            connfd = accept(listenfd, (struct sockaddr *)&client_addr, &client_addr_len);
            if (connfd < 0) {
                perror("Accept failed");
                continue;
            }

            if (fcntl(connfd, F_SETFL, fcntl(connfd, F_GETFL, 0) | O_NONBLOCK) < 0) {
                closesocket(connfd);
                perror("Set nonblock failed");
                continue;
            }

            for (i=0; i<MAX_CONN; ++i) {
                if (client[i].fd < 0) {
                    client[i].fd = connfd;
                    break;
                }
            }

            if (i >= MAX_CONN) {
                closesocket(connfd);
                puts("Too many clients.");
                continue;
            }

            client[i].events = POLLRDNORM;
            if (i > maxi) {
                maxi = i;
            }
            if (--ready_num <= 0) {
                continue;
            }
        }

        for (i=1; i<=maxi; ++i) {
            if ((connfd = client[i].fd) < 0) {
                continue;
            }
            if (client[i].revents & POLLRDNORM) {
                int ret = svrcbk((void*)(long)connfd);
                if (ret != -2) {
                    client[i].fd = -1;
                }
                if (--ready_num <= 0) {
                    break;
                }
            } else if (client[i].revents & POLLERR) {
                closesocket(connfd);
                client[i].fd = -1;
                if (--ready_num <= 0) {
                    break;
                }
            }
        }
    }
}

#ifdef __cplusplus
}
#endif


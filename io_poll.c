#include <poll.h>
#include "io_poll.h"

#ifdef __cplusplus
extern "C" {
#endif

void poll_loop(SOCKET listenfd, server_callback svrcbk)
{
    int i = 0, maxi = 0;
    struct pollfd pollev[MAX_CONN];

    pollev[0].fd = listenfd;
    pollev[0].events = POLLRDNORM;
    for (i=1; i<MAX_CONN; ++i) {
        pollev[i].fd = -1;
    }

    for (;;) {
        SOCKET connfd;
        int ready_num = poll(pollev, maxi + 1, -1);
        if (ready_num < 0) {
            perror("Poll failed");
        }

        if (pollev[0].revents == POLLRDNORM) {
            connfd = accept(listenfd, NULL, NULL);
            if (connfd < 0) {
                perror("Accept failed");
                continue;
            }

            if (fcntl(connfd, F_SETFL, fcntl(connfd, F_GETFL, 0) | O_NONBLOCK) < 0) {
                close_socket(connfd);
                perror("Set nonblock failed");
                continue;
            }

            for (i=0; i<MAX_CONN; ++i) {
                if (pollev[i].fd < 0) {
                    pollev[i].fd = connfd;
                    break;
                }
            }

            alloc_io_context((void*)(long)connfd);
            if (i >= MAX_CONN) {
                close_socket(connfd);
                free_io_context((void*)(long)(connfd));
                perror("Too many client");
                continue;
            }

            pollev[i].events = POLLRDNORM;
            if (i > maxi) {
                maxi = i;
            }
            if (--ready_num <= 0) {
                continue;
            }
        }

        for (i=1; i<=maxi; ++i) {
            if ((connfd = pollev[i].fd) < 0) {
                continue;
            }
            io_context_t *io_context = get_io_context((void*)(long)(connfd));
            if (pollev[i].revents & POLLRDNORM) {
                if (svrcbk(io_context) < 0 && get_last_error() != IO_EWOULDBLOCK) {
                    close_socket(connfd);
                    free_io_context(io_context->fd);
                    pollev[i].fd = -1;
                }
                if (--ready_num <= 0) {
                    break;
                }
            } else if (pollev[i].revents & POLLERR) {
                close_socket(connfd);
                free_io_context(io_context->fd);
                pollev[i].fd = -1;
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


#if defined USE_LIBC5 || defined USE_LIBC4
#include <sys/poll.h>
#else
#include <poll.h>
#endif
#include "io_poll.h"

void poll_loop(SOCKET listenfd, server_callback svrcbk) {
    int i = 0, maxi = 0;
    struct pollfd pollev[MAX_CONN];

    pollev[0].fd = listenfd;
    pollev[0].events = POLLIN;
    pollev[0].revents = 0;
    for (i = 1; i < MAX_CONN; ++i) {
        pollev[i].fd = INVALID_SOCKET;
        pollev[i].revents = 0;
    }

    while (1) {
        SOCKET connfd;
        int ready_num = poll(pollev, maxi + 1, -1);

        if (ready_num < 0) {
            print_error("Poll failed");
        }

        if (pollev[0].revents == POLLIN) {
            connfd = accept(listenfd, NULL, NULL);
            if (connfd == INVALID_SOCKET) {
                print_error("Accept failed");
                continue;
            }

            if (fcntl(connfd, F_SETFL, fcntl(connfd, F_GETFL, 0) | O_NONBLOCK) < 0) {
                close_socket(connfd);
                print_error("Set nonblock failed");
                continue;
            }

            for (i = 0; i < MAX_CONN; ++i) {
                if (pollev[i].fd == INVALID_SOCKET) {
                    pollev[i].fd = connfd;
                    break;
                }
            }

            if (i >= MAX_CONN) {
                close_socket(connfd);
                print_error("Too many client");
                continue;
            }

            alloc_io_context((void *)(long)connfd);

            pollev[i].events = POLLIN;
            pollev[i].revents = 0;
            if (i > maxi) {
                maxi = i;
            }
            if (--ready_num <= 0) {
                continue;
            }
        }

        for (i = 1; i <= maxi; ++i) {
            io_context_t *io_context = NULL;

            if ((connfd = pollev[i].fd) == INVALID_SOCKET) {
                continue;
            }
            io_context = get_io_context((void *)(long)(connfd));
            if (pollev[i].revents & POLLIN || pollev[i].revents & POLLOUT || pollev[i].revents & POLLHUP) {
                svrcbk(io_context);
                if (get_last_error() != IO_EWOULDBLOCK) {
                    close_socket(connfd);
                    free_io_context(io_context->fd);
                    pollev[i].fd = INVALID_SOCKET;
                } else if (io_context->sendagain) {
                    pollev[i].events = POLLIN | POLLOUT;
                } else if (!io_context->sendagain && pollev[i].events & POLLOUT) {
                    pollev[i].events = POLLIN;
                }
                if (--ready_num <= 0) {
                    break;
                }
            } else if (pollev[i].revents & POLLERR) {
                close_socket(connfd);
                free_io_context(io_context->fd);
                pollev[i].fd = INVALID_SOCKET;
                if (--ready_num <= 0) {
                    break;
                }
            }
        }
    }
}

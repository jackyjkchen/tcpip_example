#include "io_WSAPoll.h"

void WSAPoll_loop(SOCKET listenfd, server_callback svrcbk) {
    ULONG i = 0, maxi = 0;
    WSAPOLLFD pollev[MAX_CONN];

    pollev[0].fd = listenfd;
    pollev[0].events = POLLRDNORM;
    for (i = 1; i < MAX_CONN; ++i) {
        pollev[i].fd = INVALID_SOCKET;
    }

    while (1) {
        SOCKET connfd;
        int ready_num = WSAPoll(pollev, maxi + 1, -1);

        if (ready_num < 0) {
            print_error("Poll failed");
        }

        if (pollev[0].revents == POLLRDNORM) {
            u_long iMode = 1;

            connfd = accept(listenfd, NULL, NULL);
            if (connfd == INVALID_SOCKET) {
                print_error("Accept failed");
                continue;
            }

            if (ioctlsocket(connfd, FIONBIO, &iMode) != NO_ERROR) {
                print_error("Set nonblock failed");
                close_socket(connfd);
                continue;
            }

            for (i = 0; i < MAX_CONN; ++i) {
                if (pollev[i].fd == INVALID_SOCKET) {
                    pollev[i].fd = connfd;
                    break;
                }
            }

            if (i >= MAX_CONN) {
                print_error("Too many client");
                close_socket(connfd);
                continue;
            }

            alloc_io_context((void *)connfd);

            pollev[i].events = POLLRDNORM;
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
            io_context = get_io_context((void *)(connfd));
            if (pollev[i].revents & POLLRDNORM || pollev[i].revents & POLLWRNORM) {
                if (svrcbk(io_context) < 0) {
                    if (get_last_error() != IO_EWOULDBLOCK) {
                        close_socket(connfd);
                        free_io_context(io_context->fd);
                        pollev[i].fd = INVALID_SOCKET;
                    } else if (io_context->sendagain) {
                        pollev[i].events = POLLRDNORM | POLLWRNORM;
                    }
                } else if (pollev[i].events & POLLWRNORM) {
                    pollev[i].events = POLLRDNORM;
                }
                if (--ready_num <= 0) {
                    break;
                }
            } else if (pollev[i].revents & POLLERR || pollev[i].revents & POLLHUP) {
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

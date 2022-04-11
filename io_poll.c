#include <poll.h>
#include "io_poll.h"

void poll_loop(SOCKET listenfd, server_callback svrcbk) {
    int i = 0, maxi = 0;
    struct pollfd pollev[MAX_CONN];

    pollev[0].fd = listenfd;
    pollev[0].events = POLLRDNORM;
    for (i = 1; i < MAX_CONN; ++i) {
        pollev[i].fd = -1;
    }

    while (1) {
        SOCKET connfd;
        int ready_num = poll(pollev, maxi + 1, -1);

        if (ready_num < 0) {
            print_error("Poll failed");
        }

        if (pollev[0].revents == POLLRDNORM) {
            connfd = accept(listenfd, NULL, NULL);
            if (connfd < 0) {
                print_error("Accept failed");
                continue;
            }

            if (fcntl(connfd, F_SETFL, fcntl(connfd, F_GETFL, 0) | O_NONBLOCK) < 0) {
                close_socket(connfd);
                print_error("Set nonblock failed");
                continue;
            }

            for (i = 0; i < MAX_CONN; ++i) {
                if (pollev[i].fd < 0) {
                    pollev[i].fd = connfd;
                    break;
                }
            }

            alloc_io_context((void *)(long)connfd);
            if (i >= MAX_CONN) {
                close_socket(connfd);
                free_io_context((void *)(long)(connfd));
                print_error("Too many client");
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

        for (i = 1; i <= maxi; ++i) {
            io_context_t *io_context = NULL;

            if ((connfd = pollev[i].fd) < 0) {
                continue;
            }
            io_context = get_io_context((void *)(long)(connfd));
            if (pollev[i].revents & POLLRDNORM || pollev[i].revents & POLLWRNORM) {
                if (svrcbk(io_context) < 0) {
                    if (get_last_error() != IO_EWOULDBLOCK) {
                        close_socket(connfd);
                        free_io_context(io_context->fd);
                        pollev[i].fd = -1;
                    } else if (io_context->sendagain) {
                        pollev[i].events = POLLRDNORM | POLLWRNORM;
                    }
                } else if (pollev[i].events & POLLWRNORM) {
                    pollev[i].events = POLLRDNORM;
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

#include <sys/devpoll.h>
#include "io_devpoll.h"

int devpoll_loop(SOCKET listenfd, server_callback svrcbk) {
    int i = 0, dvpfd;
    struct pollfd pollev[MAX_CONN], ev;
    struct dvpoll dvpollev;

    for (i = 0; i < MAX_CONN; ++i) {
        pollev[i].fd = INVALID_SOCKET;
        pollev[i].events = 0;
        pollev[i].revents = 0;
    }

    if ((dvpfd = open("/dev/poll", O_RDWR)) < 0) {
        print_error("Open /dev/poll failed");
        return -1;
    }

    ev.fd = listenfd;
    ev.events = POLLIN;
    ev.revents = 0;
    if (write(dvpfd, &ev, sizeof(struct pollfd)) != sizeof(struct pollfd)) {
        print_error("Write /dev/poll failed");
        return -1;
    }

    while (1) {
        int ready_num = 0;
        dvpollev.dp_timeout = -1;
        dvpollev.dp_nfds = MAX_CONN;
        dvpollev.dp_fds = pollev;
        ready_num = ioctl(dvpfd, DP_POLL, &dvpollev);

        if (ready_num < 0) {
            print_error("/dev/poll failed");
        }

        for (i = 0; i < ready_num; ++i) {
            SOCKET connfd;
            io_context_t *io_context = get_io_context((void *)(long)(dvpollev.dp_fds[i].fd));

            if (dvpollev.dp_fds[i].fd == listenfd) {
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

                alloc_io_context((void *)(long)connfd);

                ev.fd = connfd;
                ev.events = POLLIN | POLLOUT;
                ev.revents = 0;
                if (write(dvpfd, &ev, sizeof(struct pollfd)) != sizeof(struct pollfd)) {
                    print_error("Write /dev/poll failed");
                    continue;
                }
            } else if (dvpollev.dp_fds[i].revents & POLLIN || dvpollev.dp_fds[i].revents & POLLOUT || dvpollev.dp_fds[i].revents & POLLHUP) {
                svrcbk(io_context);
                if (get_last_error() != IO_EWOULDBLOCK) {
                    close_socket(dvpollev.dp_fds[i].fd);
                    free_io_context(io_context->fd);
                    ev.fd = dvpollev.dp_fds[i].fd;
                    ev.events = POLLREMOVE;
                    ev.revents = 0;
                    if (write(dvpfd, &ev, sizeof(struct pollfd)) != sizeof(struct pollfd)) {
                        print_error("Write /dev/poll failed");
                        continue;
                    }
                }
            } else {
                close_socket(dvpollev.dp_fds[i].fd);
                if (io_context) {
                    free_io_context(io_context->fd);
                }
                ev.fd = dvpollev.dp_fds[i].fd;
                ev.events = POLLREMOVE;
                ev.revents = 0;
                if (write(dvpfd, &ev, sizeof(struct pollfd)) != sizeof(struct pollfd)) {
                    print_error("Write /dev/poll failed");
                    continue;
                }
            }
        }
    }

    close(dvpfd);
    return 0;
}

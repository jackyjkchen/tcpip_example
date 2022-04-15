#include <sys/epoll.h>
#include "io_epoll.h"

int epoll_loop(SOCKET listenfd, server_callback svrcbk) {
    int epollfd;
    struct epoll_event ev, events[MAX_CONN];

    ev.events = EPOLLIN;
    ev.data.fd = listenfd;

    if ((epollfd = epoll_create(MAX_CONN)) < 0) {
        print_error("Create epoll failed");
        return -1;
    }

    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &ev) == -1) {
        print_error("Epoll_ctl: listenfd failed");
        close_socket(epollfd);
        return -1;
    }

    while (1) {
        int ready_num = epoll_wait(epollfd, events, MAX_CONN, -1), i = 0;

        if (ready_num < 0) {
            print_error("Epoll_wait failed");
        }

        for (i = 0; i < ready_num; i++) {
            io_context_t *io_context = get_io_context((void *)(long)(events[i].data.fd));

            if (events[i].data.fd == listenfd) {
                SOCKET connfd = accept(listenfd, NULL, NULL);

                if (connfd == INVALID_SOCKET) {
                    print_error("Accept failed");
                    continue;
                }
                if (fcntl(connfd, F_SETFL, fcntl(connfd, F_GETFL, 0) | O_NONBLOCK) < 0) {
                    print_error("Set nonblock failed");
                    close_socket(connfd);
                    continue;
                }
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = connfd;
                if (epoll_ctl(epollfd, EPOLL_CTL_ADD, connfd, &ev) == -1) {
                    print_error("Epoll_ctl: add connfd failed");
                    close_socket(connfd);
                    continue;
                }
                alloc_io_context((void *)(long)connfd);
            } else if (events[i].events & EPOLLIN || events[i].events & EPOLLOUT) {
                if (svrcbk(io_context) < 0) {
                    if (get_last_error() != IO_EWOULDBLOCK) {
                        close_socket(events[i].data.fd);
                        free_io_context(io_context->fd);
                    } else if (io_context->sendagain && !(events[i].events & EPOLLOUT)) {
                        ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
                        ev.data.fd = events[i].data.fd;
                        if (epoll_ctl(epollfd, EPOLL_CTL_MOD, events[i].data.fd, &ev) == -1) {
                            close_socket(events[i].data.fd);
                            free_io_context(io_context->fd);
                            print_error("Epoll_ctl: mod connfd failed");
                        }
                    }
                } else if (events[i].events & EPOLLOUT) {
                    ev.events = EPOLLIN | EPOLLET;
                    ev.data.fd = events[i].data.fd;
                    if (epoll_ctl(epollfd, EPOLL_CTL_MOD, events[i].data.fd, &ev) == -1) {
                        close_socket(events[i].data.fd);
                        free_io_context(io_context->fd);
                        print_error("Epoll_ctl: mod connfd failed");
                    }
                }
            } else {
                close_socket(events[i].data.fd);
                free_io_context(io_context->fd);
            }
        }
    }
    close_socket(epollfd);
    return 0;
}

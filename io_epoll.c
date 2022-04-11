#include <sys/epoll.h>
#include "io_epoll.h"

#ifdef __cplusplus
extern "C" {
#endif

int epoll_loop(SOCKET listenfd, server_callback svrcbk)
{
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

    for (;;) {
        int ready_num = epoll_wait(epollfd, events, MAX_CONN, -1), i = 0;
        if (ready_num < 0) {
            print_error("Epoll_wait failed");
        }

        for (i=0; i<ready_num; i++) {
            io_context_t *io_context = get_io_context((void*)(long)(events[i].data.fd));
            if (events[i].data.fd == listenfd) {
                SOCKET connfd = accept(listenfd, NULL, NULL);
                if (connfd < 0) {
                    print_error("Accept failed");
                    continue;
                }
                if (fcntl(connfd, F_SETFL, fcntl(connfd, F_GETFL, 0) | O_NONBLOCK) < 0) {
                    close_socket(connfd);
                    print_error("Set nonblock failed");
                    continue;
                }
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = connfd;
                if (epoll_ctl(epollfd, EPOLL_CTL_ADD, connfd, &ev) == -1) {
                    close_socket(connfd);
                    print_error("Epoll_ctl: add connfd failed");
                    continue;
                }
                alloc_io_context((void*)(long)connfd);
            } else if (events[i].events & EPOLLIN) {
                if (svrcbk(io_context) < 0 && get_last_error() != IO_EWOULDBLOCK) {
                    close_socket(events[i].data.fd);
                    free_io_context(io_context->fd);
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

#ifdef __cplusplus
}
#endif

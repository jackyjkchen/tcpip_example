#include <sys/epoll.h>
#include "epoll_impl.h"

#ifdef __cplusplus
extern "C" {
#endif

int epoll_loop(int listenfd, server_callback svrcbk)
{
    int epollfd, connfd, ready_num, i;
    socklen_t client_addr_len;
    struct sockaddr_in client_addr;
    struct epoll_event ev, events[MAX_CONN];
    ev.events = EPOLLIN;
    ev.data.fd = listenfd;

    if ((epollfd = epoll_create(MAX_CONN)) < 0) {
        perror("Create epoll failed");
        return -1;
    }

    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &ev) == -1) {
        perror("Epoll_ctl: listenfd failed");
        close(epollfd);
        return -1;
    }

    for (;;) {
        ready_num = epoll_wait(epollfd, events, MAX_CONN, -1);
        if (ready_num < 0) {
            perror("Epoll_wait failed");
        }

        for (i=0; i<ready_num; i++) {
            if (events[i].data.fd == listenfd) {
                client_addr_len = sizeof(client_addr);
                connfd = accept(listenfd, (struct sockaddr *)&client_addr, &client_addr_len);
                if (connfd < 0) {
                    perror("Accept failed");
                    continue;
                }
                if (fcntl(connfd, F_SETFL, fcntl(connfd, F_GETFL, 0) | O_NONBLOCK) < 0) {
                    close(connfd);
                    perror("Set nonblock failed");
                    continue;
                }
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = connfd;
                if (epoll_ctl(epollfd, EPOLL_CTL_ADD, connfd, &ev) == -1) {
                    close(connfd);
                    perror("Epoll_ctl: add connfd failed");
                    continue;
                }
            } else if (events[i].events & EPOLLIN) {
                svrcbk((void*)(long)events[i].data.fd);
            } else {
                close(events[i].data.fd);
            }
        }
    }
    close(epollfd);
    return 0;
}

#ifdef __cplusplus
}
#endif

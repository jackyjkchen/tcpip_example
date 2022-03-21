#include <sys/types.h>
#include <sys/event.h>
#include "epoll_impl.h"

#ifdef __cplusplus
extern "C" {
#endif

int kqueue_loop(int listenfd, server_callback svrcbk)
{
    int connfd, ready_num, kq, i;
    socklen_t client_addr_len;
    struct sockaddr_in client_addr;
    struct kevent kev, kev_list[MAX_CONN];
    struct timespec ts;

    if ((kq = kqueue()) < 0) {
        close(listenfd);
        perror("Create kqueue failed");
        return -1;
    }

    EV_SET(&kev, listenfd, EVFILT_READ, EV_ADD | EV_CLEAR, 0, 0, 0);
    ts.tv_sec = ts.tv_nsec = 0;
    kevent(kq, &kev, 1, NULL, 0, &ts);

    for (;;) {
        ready_num = kevent(kq, NULL, 0, kev_list, MAX_CONN, NULL);
        if (ready_num < 0) {
            perror("Kevent failed");
        }

        for (i=0; i<ready_num; i++) {
            if ((int)(kev_list[i].ident) == listenfd) {
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
                EV_SET(&kev, connfd, EVFILT_READ, EV_ADD | EV_CLEAR, 0, 0, 0);
                if (kevent(kq, &kev, 1, NULL, 0, NULL) != 0) {
                    close(connfd);
                    perror("Add kevent failed");
                    continue;
                }
            } else if (kev_list[i].filter == EVFILT_READ) {
                svrcbk((void*)(long)kev_list[i].ident);
            } else {
                close(kev_list[i].ident);
            }
        }
    }

    close(kq);
}

#ifdef __cplusplus
}
#endif

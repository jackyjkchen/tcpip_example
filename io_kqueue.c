#include <sys/types.h>
#include <sys/time.h>
#include <sys/event.h>
#include "io_kqueue.h"

#ifdef __cplusplus
extern "C" {
#endif

int kqueue_loop(SOCKET listenfd, server_callback svrcbk)
{
    SOCKET connfd;
    int kq, i;
    socklen_t client_addr_len;
    struct sockaddr_in client_addr;
    struct kevent kev, kev_list[MAX_CONN];
    struct timespec ts;

    if ((kq = kqueue()) < 0) {
        closesocket(listenfd);
        perror("Create kqueue failed");
        return -1;
    }

    EV_SET(&kev, listenfd, EVFILT_READ, EV_ADD | EV_CLEAR, 0, 0, 0);
    ts.tv_sec = ts.tv_nsec = 0;
    kevent(kq, &kev, 1, NULL, 0, &ts);

    for (;;) {
        int ready_num = kevent(kq, NULL, 0, kev_list, MAX_CONN, NULL);
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
                    closesocket(connfd);
                    perror("Set nonblock failed");
                    continue;
                }
                EV_SET(&kev, connfd, EVFILT_READ, EV_ADD | EV_CLEAR, 0, 0, 0);
                if (kevent(kq, &kev, 1, NULL, 0, NULL) != 0) {
                    closesocket(connfd);
                    perror("Add kevent failed");
                    continue;
                }
            } else if (kev_list[i].filter == EVFILT_READ) {
                svrcbk((void*)(long)kev_list[i].ident);
            } else {
                closesocket(kev_list[i].ident);
            }
        }
    }

    closesocket(kq);
    return 0;
}

#ifdef __cplusplus
}
#endif

#include <sys/types.h>
#include <sys/time.h>
#include <sys/event.h>
#include "io_kqueue.h"

#ifdef __cplusplus
extern "C" {
#endif

int kqueue_loop(SOCKET listenfd, server_callback svrcbk)
{
    int kq, i;
    struct kevent kev, kevents[MAX_CONN];
    struct timespec ts;

    if ((kq = kqueue()) < 0) {
        close_socket(listenfd);
        perror("Create kqueue failed");
        return -1;
    }

    EV_SET(&kev, listenfd, EVFILT_READ, EV_ADD | EV_CLEAR, 0, 0, 0);
    ts.tv_sec = ts.tv_nsec = 0;
    kevent(kq, &kev, 1, NULL, 0, &ts);

    for (;;) {
        int ready_num = kevent(kq, NULL, 0, kevents, MAX_CONN, NULL);
        if (ready_num < 0) {
            perror("Kevent failed");
        }

        for (i=0; i<ready_num; i++) {
            io_context_t *io_context = get_io_context((void*)(long)(kevents[i].ident));
            if ((int)(kevents[i].ident) == listenfd) {
                SOCKET connfd = accept(listenfd, NULL, NULL);
                if (connfd < 0) {
                    perror("Accept failed");
                    continue;
                }
                if (fcntl(connfd, F_SETFL, fcntl(connfd, F_GETFL, 0) | O_NONBLOCK) < 0) {
                    close_socket(connfd);
                    perror("Set nonblock failed");
                    continue;
                }
                EV_SET(&kev, connfd, EVFILT_READ, EV_ADD | EV_CLEAR, 0, 0, 0);
                if (kevent(kq, &kev, 1, NULL, 0, NULL) != 0) {
                    close_socket(connfd);
                    perror("Add kevent failed");
                    continue;
                }
                alloc_io_context((void*)(long)connfd);
            } else if (kevents[i].filter == EVFILT_READ) {
                if (svrcbk(io_context) < 0 && get_last_error() != IO_EWOULDBLOCK) {
                    close_socket(kevents[i].ident);
                    free_io_context(io_context->fd);
                }
            } else {
                close_socket(kevents[i].ident);
                free_io_context(io_context->fd);
            }
        }
    }

    close_socket(kq);
    return 0;
}

#ifdef __cplusplus
}
#endif

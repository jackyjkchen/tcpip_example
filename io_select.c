#include <sys/time.h>
#ifndef _WIN32
#include <sys/select.h>
#endif
#include "io_select.h"

#ifdef __cplusplus
extern "C" {
#endif

void select_loop(SOCKET listenfd, server_callback svrcbk)
{
    SOCKET connfd, maxfd;
    ssize_t selfd[MAX_CONN];
    int ready_num;
    int i = 0, maxi = -1;
    fd_set allset, rset;

    maxfd = listenfd;
    for (i=0; i<MAX_CONN; ++i) {
        selfd[i] = -1;
    }
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);

    for (;;) {
        rset = allset;
        ready_num = select(maxfd+1, &rset, NULL, NULL, NULL);
        if (ready_num < 0) {
            perror("Select failed");
        }

        if (FD_ISSET(listenfd, &rset)) {
#ifdef _WIN32
            u_long iMode = 1;
#endif
            connfd = accept(listenfd, NULL, NULL);
            if (connfd < 0) {
                perror("Accept failed");
                continue;
            }

#ifdef _WIN32
            if (ioctlsocket(listenfd, FIONBIO, &iMode) != NO_ERROR ) {
#else
            if (fcntl(connfd, F_SETFL, fcntl(connfd, F_GETFL, 0) | O_NONBLOCK) < 0) {
#endif
                close_socket(connfd);
                perror("Set nonblock failed");
                continue;
            }

            for (i=0; i<MAX_CONN; ++i) {
                if (selfd[i] < 0) {
                    selfd[i] = connfd;
                    break;
                }
                printf("%d\n",i);
            }

#ifdef _WIN32
            alloc_io_context((void*)connfd);
#else
            alloc_io_context((void*)(long)connfd);
#endif
            if (i >= MAX_CONN) {
                close_socket(connfd);
#ifdef _WIN32
                free_io_context((void*)(connfd));
#else
                free_io_context((void*)(long)(connfd));
#endif
                perror("Too many clients");
                continue;
            }

            FD_SET(connfd, &allset);
            if (connfd > maxfd) {
                maxfd = connfd;
            }
            if (i > maxi) {
                maxi = i;
            }
            if (--ready_num <= 0) {
                continue;
            }
        }

        for (i=0; i<=maxi; ++i) {
            if ((connfd = selfd[i]) < 0) {
                continue;
            }
            if (FD_ISSET(connfd, &rset)) {
#ifdef _WIN32
                io_context_t *io_context = get_io_context((void*)(connfd));
#else
                io_context_t *io_context = get_io_context((void*)(long)(connfd));
#endif
                if (svrcbk(io_context) < 0 && get_last_error() != IO_EWOULDBLOCK) {
                    FD_CLR(connfd, &allset);
                    close_socket(connfd);
                    free_io_context(io_context->fd);
                    selfd[i] = -1;
                }
                if (--ready_num <= 0) {
                    break;
                }
            }
        }
    }
}

#ifdef __cplusplus
}
#endif


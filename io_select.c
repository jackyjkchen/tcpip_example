#ifndef _WIN32
#if defined USE_LIBC5 || defined USE_LIBC4
#include <sys/time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>
#endif
#endif
#include "io_select.h"

void select_loop(SOCKET listenfd, server_callback svrcbk) {
    SOCKET connfd, maxfd;
    SOCKET selfd[FD_SETSIZE];
    int ready_num;
    int i = 0, maxi = -1;
    fd_set allset, rset;

    maxfd = listenfd;
    for (i = 0; i < FD_SETSIZE; ++i) {
        selfd[i] = INVALID_SOCKET;
    }
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);

    while (1) {
        rset = allset;
        ready_num = select(maxfd + 1, &rset, NULL, NULL, NULL);
        if (ready_num < 0) {
            print_error("Select failed");
        }

        if (FD_ISSET(listenfd, &rset)) {
#ifdef _WIN32
            u_long iMode = 1;
#endif
            connfd = accept(listenfd, NULL, NULL);
            if (connfd == INVALID_SOCKET) {
                print_error("Accept failed");
                continue;
            }
#ifdef _WIN32
            if (ioctlsocket(listenfd, FIONBIO, &iMode) != NO_ERROR) {
#else
            if (fcntl(connfd, F_SETFL, fcntl(connfd, F_GETFL, 0) | O_NONBLOCK) < 0) {
#endif
                print_error("Set nonblock failed");
                close_socket(connfd);
                continue;
            }

            for (i = 0; i < FD_SETSIZE; ++i) {
                if (selfd[i] == INVALID_SOCKET) {
                    selfd[i] = connfd;
                    break;
                }
            }

            if (i >= FD_SETSIZE) {
                print_error("Too many clients");
                close_socket(connfd);
                continue;
            }
#ifdef _WIN32
            alloc_io_context((void *)connfd);
#else
            alloc_io_context((void *)(long)connfd);
#endif

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

        for (i = 0; i <= maxi; ++i) {
            if ((connfd = selfd[i]) == INVALID_SOCKET) {
                continue;
            }
            if (FD_ISSET(connfd, &rset)) {
#ifdef _WIN32
                io_context_t *io_context = get_io_context((void *)(connfd));
#else
                io_context_t *io_context = get_io_context((void *)(long)(connfd));
#endif
                svrcbk(io_context);
                if (get_last_error() != IO_EWOULDBLOCK) {
                    FD_CLR(connfd, &allset);

                    close_socket(connfd);
                    free_io_context(io_context->fd);
                    selfd[i] = INVALID_SOCKET;
                }
                if (--ready_num <= 0) {
                    break;
                }
            }
        }
    }
}

#include <sys/time.h>
#ifndef _WIN32
#include <sys/resource.h>
#endif
#include "io_common.h"

#ifdef __cplusplus
extern "C" {
#endif

ssize_t recvn(SOCKET fd, void *buf, size_t buf_size)
{
    size_t nleft = buf_size;
    ssize_t nrecv = 0;
    char *p = (char *)buf;

    while (nleft > 0){
        if ((nrecv = recv(fd, p, nleft, 0)) < 0) {
            if (errno == EINTR) {
                nrecv = 0;
            } else if (errno == EAGAIN) {
                return nleft - buf_size;
            } else {
                return -1;
            }
        } else if (nrecv == 0){
            break;
        }

        nleft -= nrecv;
        p += nrecv;
    }

    return buf_size - nleft;
}

ssize_t sendn(SOCKET fd, const void *buf, size_t buf_size)
{
    size_t nleft = buf_size;
    ssize_t nsend = 0;
    const char *p = (const char *)buf;

    while (nleft > 0) {
        if ((nsend = send(fd, p, nleft, 0)) <= 0) {
            if (nsend < 0 && errno == EINTR) {
                nsend = 0;
            } else if (nsend < 0 && errno == EAGAIN) {
                return nleft - buf_size;
            } else {
                return -1;
            }
        }

        nleft -= nsend;
        p += nsend;
    }
    return buf_size - nleft;
}

#ifndef _WIN32
int closesocket(int fd) {
    return close(fd);
}

int set_rlimit()
{
    struct rlimit rl;
    rl.rlim_cur = 65536;
    rl.rlim_max = 65536;
    return setrlimit(RLIMIT_NOFILE, &rl);
}
#endif

#ifdef __cplusplus
}
#endif


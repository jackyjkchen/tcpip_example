#include <sys/resource.h>
#include "io_common.h"

#ifdef __cplusplus
extern "C" {
#endif

ssize_t readn(int fd, void *buf, size_t buf_size)
{
    size_t nleft = buf_size;
    ssize_t nread = 0;
    unsigned char *p = (unsigned char *)buf;

    while (nleft > 0){
        if ((nread = read(fd, p, nleft)) < 0) {
            if (errno == EINTR) {
                nread = 0;
            } else if (errno == EWOULDBLOCK) {
                return nleft - buf_size;
            } else {
                return -1;
            }
        } else if (nread == 0){
            break;
        }

        nleft -= nread;
        p += nread;
    }

    return buf_size - nleft;
}

ssize_t writen(int fd, const void *buf, size_t buf_size)
{
    size_t nleft = buf_size;
    ssize_t nwritten = 0;
    const unsigned char *p = (const unsigned char *)buf;

    while (nleft > 0) {
        if ((nwritten = write(fd, p, nleft)) <= 0) {
            if (nwritten < 0 && errno == EINTR) {
                nwritten = 0;
            } else if (nwritten < 0 && errno == EWOULDBLOCK) {
                return nleft - buf_size;
            } else {
                return -1;
            }
        }

        nleft -= nwritten;
        p += nwritten;
    }
    return buf_size - nleft;
}

int set_rlimit()
{
    struct rlimit rl;
    rl.rlim_cur = 65536;
    rl.rlim_max = 65536;
    return setrlimit(RLIMIT_NOFILE, &rl);
}

#ifdef __cplusplus
}
#endif


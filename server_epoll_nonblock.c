#include "io_epoll.h"

int main(int argc, char **argv)
{
    int listenfd, ret;

    listenfd = server_socket_init(1);
    if (listenfd < 0) {
        return -1;
    }

    ret = epoll_loop(listenfd, reflect_server_callback);

    close(listenfd);
    return ret;
}

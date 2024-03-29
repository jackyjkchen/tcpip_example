#include "io_epoll.h"

int main() {
    SOCKET listenfd;
    int ret;

    listenfd = server_socket_init(TCP, 1);
    if (listenfd == INVALID_SOCKET) {
        return -1;
    }

    ret = epoll_loop(listenfd, reflect_server_callback);

    close_socket(listenfd);
    return ret;
}

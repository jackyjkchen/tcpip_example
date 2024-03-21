#include "io_kqueue.h"

int main() {
    SOCKET listenfd;
    int ret = 0;

    listenfd = server_socket_init(TCP, 1);
    if (listenfd == INVALID_SOCKET) {
        return -1;
    }

    ret = kqueue_loop(listenfd, reflect_server_callback);

    close_socket(listenfd);
    return ret;
}

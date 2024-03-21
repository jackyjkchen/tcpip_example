#include "io_accept.h"

int main() {
    SOCKET listenfd, ret = 0;

    listenfd = server_socket_init(TCP, 0);
    if (listenfd == INVALID_SOCKET) {
        return -1;
    }

    ret = accept_loop(listenfd, reflect_server_callback);

    close_socket(listenfd);
    return ret;
}

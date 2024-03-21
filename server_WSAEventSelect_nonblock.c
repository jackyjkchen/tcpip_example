#include "io_WSAEventSelect.h"

int main() {
    SOCKET listenfd;
    int ret;

    listenfd = server_socket_init(TCP, 1);
    if (listenfd == INVALID_SOCKET) {
        return -1;
    }

    ret = WSAEventSelect_loop(listenfd, reflect_server_callback);

    close_socket(listenfd);
    WSACleanup();
    return ret;
}

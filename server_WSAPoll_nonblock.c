#include "io_WSAPoll.h"

int main() {
    SOCKET listenfd;

    listenfd = server_socket_init(TCP, 1);
    if (listenfd == INVALID_SOCKET) {
        return -1;
    }

    WSAPoll_loop(listenfd, reflect_server_callback);

    close_socket(listenfd);
    WSACleanup();
    return 0;
}

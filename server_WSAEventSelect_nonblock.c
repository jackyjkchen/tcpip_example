#include "io_WSAEventSelect.h"

int main(int argc, char **argv) {
    SOCKET listenfd;
    int ret;

    listenfd = server_socket_init(TCP, 1);
    if (listenfd < 0) {
        return -1;
    }

    ret = WSAEventSelect_loop(listenfd, reflect_server_callback);

    close_socket(listenfd);
    WSACleanup();
    return ret;
}

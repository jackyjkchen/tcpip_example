#include "io_accept.h"

int main(int argc, char **argv) {
    SOCKET listenfd, ret = 0;

    listenfd = server_socket_init(TCP, 0);
    if (listenfd < 0) {
        return -1;
    }

    ret = accept_loop(listenfd, reflect_server_callback);

    close_socket(listenfd);
    return ret;
}

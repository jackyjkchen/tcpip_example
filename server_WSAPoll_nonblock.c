#include "io_WSAPoll.h"

int main(int argc, char **argv) {
    SOCKET listenfd;

    listenfd = server_socket_init(TCP, 1);
    if (listenfd < 0) {
        return -1;
    }

    WSAPoll_loop(listenfd, reflect_server_callback);

    close_socket(listenfd);
    return 0;
}

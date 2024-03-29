#include "io_poll.h"

#define MAX_CONN  10240

int main() {
    SOCKET listenfd;

    listenfd = server_socket_init(TCP, 1);
    if (listenfd == INVALID_SOCKET) {
        return -1;
    }

    poll_loop(listenfd, reflect_server_callback);

    close_socket(listenfd);
    return 0;
}

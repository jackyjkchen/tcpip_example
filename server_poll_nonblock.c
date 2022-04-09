#include "io_poll.h"

#define MAX_CONN  10240

int main(int argc, char **argv)
{
    SOCKET listenfd;

    listenfd = server_socket_init(1);
    if (listenfd < 0) {
        return -1;
    }

    poll_loop(listenfd, reflect_server_callback);

    closesocket(listenfd);
    return 0;
}

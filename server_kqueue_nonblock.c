#include "io_kqueue.h"

int main(int argc, char **argv)
{
    SOCKET listenfd;
    int ret = 0;

    listenfd = server_socket_init(1);
    if (listenfd < 0) {
        return -1;
    }

    ret = kqueue_loop(listenfd, reflect_server_callback);

    closesocket(listenfd);
    return ret;
}

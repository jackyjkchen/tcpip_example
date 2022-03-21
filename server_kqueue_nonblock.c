#include "kqueue_impl.h"

int main(int argc, char **argv)
{
    int listenfd, ret = 0;

    listenfd = server_socket_init(1);
    if (listenfd < 0) {
        return -1;
    }

    ret = kqueue_loop(listenfd, reflect_server_callback);

    close(listenfd);
    return ret;
}

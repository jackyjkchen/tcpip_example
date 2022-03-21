#include "io_common.h"

#define MAX_CONN  10240

int main(int argc, char **argv)
{
    int listenfd;

    listenfd = server_socket_init(1);
    if (listenfd < 0) {
        return -1;
    }

    poll_loop(listenfd, reflect_server_callback);

    close(listenfd);
    return 0;
}

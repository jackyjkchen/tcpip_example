#include "io_common.h"

int main(int argc, char **argv)
{
    int listenfd;

    listenfd = server_socket_init(1);
    if (listenfd < 0) {
        return -1;
    }

    select_loop(listenfd, reflect_server_callback);

    close(listenfd);
    return 0;
}

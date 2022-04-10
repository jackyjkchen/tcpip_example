#include "io_select.h"

int main(int argc, char **argv)
{
    SOCKET listenfd;

    listenfd = server_socket_init(1);
    if (listenfd < 0) {
        return -1;
    }

    select_loop(listenfd, reflect_server_callback);

    close_socket(listenfd);
    return 0;
}

#include "io_server.h"

int main(int argc, char **argv) {
    SOCKET fd;
    char buf[UDP_BUF_SIZE] = { 0 };

    fd = server_socket_init(UDP, 0);
    if (fd == INVALID_SOCKET) {
        return -1;
    }

    while (1) {
        struct sockaddr client_addr;
        socklen_t socklen = sizeof(client_addr);
        ssize_t n = 0;

        memset(&client_addr, 0x00, socklen);
        if ((n = recvfrom(fd, buf, UDP_BUF_SIZE, 0, &client_addr, &socklen)) >= 0) {
            if (sendto(fd, buf, n, MSG_NOSIGNAL, &client_addr, sizeof(client_addr)) < 0) {
                print_error("Send failed");
            }
        } else {
            print_error("Recv failed");
        }
    }

    close_socket(fd);
    return 0;
}

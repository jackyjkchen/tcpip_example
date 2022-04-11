#include "io_client.h"

int main(int argc, char **argv) {
    int client_num, i;
    struct sockaddr_in server_addr;
    socklen_t socklen = sizeof(struct sockaddr);
    SOCKET fd;
    const char *str = "hello, world";
    char buf[BUF_SIZE] = { 0 };

    if (argc != 3) {
        fprintf(stderr, "Please input server adderss and client num.");
        return -1;
    }

    client_num = strtol(argv[2], (char **)NULL, 10);
    if (client_num <= 0) {
        print_error("invalid client num");
        return -1;
    }

    if (client_socket_init(argv[1], &server_addr) != 0) {
        return -1;
    }

    fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (fd < 0) {
        print_error("Create socket failed");
        return -1;
    }

    memcpy(buf, str, strlen(str) + 1);
    for (i = 0; i < client_num; ++i) {
        if (sendto(fd, buf, BUF_SIZE, MSG_NOSIGNAL,
                   (const struct sockaddr *)&server_addr, sizeof(server_addr)) == BUF_SIZE) {
            if (recvfrom(fd, buf, BUF_SIZE, 0, (struct sockaddr *)&server_addr, &socklen) == BUF_SIZE) {
                printf("send and recv: %ld bytes - string: %s\n", (long)(BUF_SIZE), buf);
            } else {
                print_error("Recv failed");
            }
        } else {
            print_error("Send failed");
        }
    }

    close_socket(fd);
    return 0;
}

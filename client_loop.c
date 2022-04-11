#include "io_client.h"

int main(int argc, char **argv)
{
    int client_num, i;
    struct sockaddr_in server_addr;

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

    for (i=0; i<client_num; ++i) {
        reflect_client_callback(&server_addr);
    }

    return 0;
}

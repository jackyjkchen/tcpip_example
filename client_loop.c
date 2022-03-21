#include "io_client.h"

int main(int argc, char **argv)
{
    int client_num, i;
    struct sockaddr_in server_addr;

    if (argc != 3) {
        printf("Please input server adderss and client num.");
        return -1;
    }

    client_num = strtol(argv[2], (char **)NULL, 10);
    if (client_num <= 0) {
        perror("invalid client num");
        return -1;
    }

    if (set_rlimit() != 0) {
        perror("Set rlimit failed");
    }

    memset(&server_addr, 0x00, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERV_PORT);
    if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) < 0) {
        perror("Server address invalid");
        return -1;
    }

    for (i=0; i<client_num; ++i) {
        reflect_client_callback(&server_addr);
    }

    return 0;
}

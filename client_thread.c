#include "io_client.h"
#include "thread_pool_c.h"

int main(int argc, char **argv) {
    int client_num, i;
    struct sockaddr_in server_addr;
    struct THREADPOOL_CTX thrd_ctx;

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

    if (threadpool_init(&thrd_ctx, 0) != 0) {
        print_error("threadpool_init failed");
        return -1;
    }

    for (i = 0; i < client_num; ++i) {
        threadpool_addtask(&thrd_ctx, reflect_client_callback, &server_addr);
    }

    threadpool_waitallthrd(&thrd_ctx);
    threadpool_clean(&thrd_ctx);

    return 0;
}

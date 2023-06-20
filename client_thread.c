#include "io_client.h"
#include "thread_pool_c.h"

int main(int argc, char **argv) {
    int client_num, i;
    struct sockaddr_in server_addr;
    THREADPOOL_CTX thrd_ctx;

    if (argc != 3) {
        fprintf(stderr, "Please input server adderss and client num.");
        return -1;
    }

    client_num = strtol(argv[2], (char **)NULL, 10);
    if (client_num <= 0) {
        print_error("invalid client num");
        return -1;
    }

    if (client_socket_init(argv[1], TCP_SERV_PORT, &server_addr) != 0) {
        return -1;
    }

    memset(&thrd_ctx, 0x00, sizeof(THREADPOOL_CTX));
    if (!threadpool_startup(&thrd_ctx, 0)) {
        print_error("threadpool_startup failed");
        return -1;
    }

    for (i = 0; i < client_num; ++i) {
        while (!threadpool_addtask(&thrd_ctx, reflect_client_callback, &server_addr)) {
#ifdef _WIN32
            Sleep(0);
#else
            sleep(0);
#endif
        }
    }

    threadpool_waitallthrd(&thrd_ctx);
    threadpool_cleanup(&thrd_ctx);

    return 0;
}

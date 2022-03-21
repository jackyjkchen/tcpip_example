#include "io_server.h"

#ifdef __cplusplus
extern "C" {
#endif

int server_socket_init(int nonblock)
{
    int listenfd;
    int reuse_addr = 1;
    struct sockaddr_in server_addr;
    if (set_rlimit() != 0) {
        perror("Set rlimit failed");
        return -1;
    }

    listenfd = socket(PF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        perror("Create socket failed");
        return -1;
    }

    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr)) != 0) {
        perror("Set SO_REUSEADDR failed");
        return -1;
    }

    memset(&server_addr, 0x00, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERV_PORT);

    if (bind(listenfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) != 0) {
        close(listenfd);
        perror("Bind server_addr failed");
        return -1;
    }

    if (listen(listenfd, LISTENQ) != 0) {
        close(listenfd);
        perror("Listen port failed");
        return -1;
    }

    if (nonblock) {
        if (fcntl(listenfd, F_SETFL, fcntl(listenfd, F_GETFL, 0) | O_NONBLOCK) < 0) {
            close(listenfd);
            perror("Set nonblock failed");
            return -1;
        }
    }
    return listenfd;
}

int reflect_server_callback(void *param)
{
    ssize_t n = 0;
    int ret = 0;
    int connfd = (int)(long)param;
    unsigned char buf[BUF_SIZE];
    while (1) {
        n = readn(connfd, buf, BUF_SIZE);
        if (n < 0 && errno != EWOULDBLOCK) {
            perror("Recv failed");
            close(connfd);
            ret = -1;
            break;
        } else {
            ssize_t w = writen(connfd, buf, n>=0?n:-n);
            if (w < 0 && errno != EWOULDBLOCK) {
                perror("Send failed");
                close(connfd);
                ret = -1;
                break;
            }
            if (n >= 0 && n != BUF_SIZE) {
                shutdown(connfd, SHUT_WR);
                close(connfd);
            }
        }
        if((n>=0?n:-n) != BUF_SIZE) {
            if (n < 0) {
                ret = -2;
            }
            break;
        }
    }
    return ret;
}

#ifdef __cplusplus
}
#endif


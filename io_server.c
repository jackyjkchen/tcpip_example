#include "io_server.h"

#ifdef __cplusplus
extern "C" {
#endif

int server_socket_init(int nonblock)
{
    SOCKET listenfd;
    int reuse_addr = 1;
    struct sockaddr_in server_addr;
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        perror("WSAStartup failed");
        return -1;
    }
#else
    if (set_rlimit() != 0) {
        perror("Set rlimit failed");
        return -1;
    }
#endif

    listenfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenfd < 0) {
        perror("Create socket failed");
        return -1;
    }

    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuse_addr, sizeof(reuse_addr)) != 0) {
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
#ifdef _WIN32
        u_long iMode = 0;
        if (ioctlsocket(listenfd, FIONBIO, &iMode) != NO_ERROR ) {
#else
        if (fcntl(listenfd, F_SETFL, fcntl(listenfd, F_GETFL, 0) | O_NONBLOCK) < 0) {
#endif
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
#ifdef _WIN32
    SOCKET connfd = (SOCKET)param;
#else
    int connfd = (int)(long)param;
#endif
    unsigned char buf[BUF_SIZE];
    while (1) {
        n = recvn(connfd, buf, BUF_SIZE);
        if (n < 0 && errno != EAGAIN) {
            perror("Recv failed");
            close(connfd);
            ret = -1;
            break;
        } else {
            ssize_t w = sendn(connfd, buf, n>=0?n:-n);
            if (w < 0 && errno != EAGAIN) {
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


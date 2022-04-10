#include "io_client.h"

#ifdef __cplusplus
extern "C" {
#endif

int client_socket_init(const char *straddr, struct sockaddr_in *pserver_addr) {
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

    memset(pserver_addr, 0x00, sizeof(struct sockaddr_in));
    pserver_addr->sin_family = AF_INET;
    pserver_addr->sin_port = htons(SERV_PORT);
#ifdef _WIN32
    pserver_addr->sin_addr.s_addr = inet_addr(straddr);
    if (pserver_addr->sin_addr.s_addr == INADDR_NONE || pserver_addr->sin_addr.s_addr == INADDR_ANY) {
#else
    if (inet_pton(AF_INET, straddr, &pserver_addr->sin_addr) < 0) {
#endif
        perror("Server address invalid");
        return -1;
    }
    return 0;
}

void reflect_client_callback(void *param)
{
    SOCKET connfd = 0;
    unsigned char buf[BUF_SIZE] = {0};
    const char *str = "hello, world\n";
    struct sockaddr_in *pserver_addr = (struct sockaddr_in *)param;
    ssize_t bufsize = strlen(str) + 1;
    ssize_t recvbytes = 0;
    ssize_t sendbytes = 0;
    do {
        SOCKET connfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (connfd < 0) {
            perror("Create socket failed");
            break;
        }

        if (connect(connfd, (const struct sockaddr *)pserver_addr, sizeof(struct sockaddr_in)) != 0) {
            perror("Connect failed");
            break;
        }

        while(1) {
            int r = 0, w = 0;
            if (recvbytes == sendbytes) {
                w = send(connfd, buf + sendbytes, bufsize - sendbytes, MSG_NOSIGNAL);
            }   
            if (w < 0) {
                perror("Send failed");
                break;
            } else {
                sendbytes += w;
                if (sendbytes == BUF_SIZE) {
                    shutdown(connfd, IO_SHUT_WR);
                }
                if ((r = recv(connfd, buf + recvbytes, bufsize - recvbytes, 0)) < 0) {
                    perror("Recv failed");
                    break;
                } else {
                    recvbytes += r;
                    if (recvbytes >= bufsize) {
                        shutdown(connfd, IO_SHUT_RD);
                        printf("%s", buf);
                        break;
                    }
                }
            }
        }
    } while(0);

    if (connfd > 0) {
        close_socket(connfd);
    }
}

#ifdef __cplusplus
}
#endif


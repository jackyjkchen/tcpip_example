#include "io_client.h"

int client_socket_init(const char *straddr, struct sockaddr_in *pserver_addr) {
#ifdef _WIN32
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        print_error("WSAStartup failed");
        return -1;
    }
#else
    if (set_rlimit() != 0) {
        print_error("Set rlimit failed");
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
        print_error("Server address invalid");
        return -1;
    }
    return 0;
}

void reflect_client_callback(void *param) {
    SOCKET connfd = INVALID_SOCKET;
    const char *str = "hello, world";
    char *buf = NULL;
    struct sockaddr_in *pserver_addr = (struct sockaddr_in *)param;
    ssize_t bufsize = TCP_DATA_SIZE;
    ssize_t recvbytes = 0;
    ssize_t sendbytes = 0;

    buf = (char *)malloc(TCP_DATA_SIZE);
    if (buf == NULL) {
        print_error("Malloc buf failed");
        return;
    }
    memset(buf, 0x00, TCP_DATA_SIZE);
    memcpy(buf, str, strlen(str) + 1);
    do {
#ifdef _WIN32
        u_long iMode = 1;
#else
        int flag;
#endif
        connfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        if (connfd == INVALID_SOCKET) {
            print_error("Create socket failed");
            break;
        }

        if (connect(connfd, (const struct sockaddr *)pserver_addr, sizeof(struct sockaddr_in)) != 0) {
            print_error("Connect failed");
            break;
        }

#ifndef _WIN32
        flag = fcntl(connfd, F_GETFL, 0);
#endif

        while (1) {
            int r = 0, w = 0, err = IO_OK;

            if (sendbytes < bufsize) {
#ifdef _WIN32
                iMode = 1;
                if (ioctlsocket(connfd, FIONBIO, &iMode) != NO_ERROR) {
#else
                if (fcntl(connfd, F_SETFL, flag | O_NONBLOCK) < 0) {
#endif
                    print_error("Set nonblock failed");
                    break;
                }
                w = send(connfd, buf + sendbytes, bufsize - sendbytes, MSG_NOSIGNAL);
            }
            if (w < 0) {
                err = get_last_error();
                if (err == IO_EINTR) {
                    continue;
                } else if (err != IO_EWOULDBLOCK) {
                    print_error("Send failed");
                    break;
                }
            }
            if (w >= 0 || (w < 0 && err == IO_EWOULDBLOCK)) {
                if (w > 0) {
                    sendbytes += w;
                    if (sendbytes >= bufsize) {
                        shutdown(connfd, IO_SHUT_WR);
                    }
                }
#ifdef _WIN32
                    iMode = 0;
                    if (ioctlsocket(connfd, FIONBIO, &iMode) != NO_ERROR) {
#else
                    if (fcntl(connfd, F_SETFL, flag) < 0) {
#endif
                        print_error("Set nonblock failed");
                        break;
                    }
                if ((r = recv(connfd, buf + recvbytes, bufsize - recvbytes, 0)) < 0) {
                    err = get_last_error();
                    if (err == IO_EINTR) {
                        continue;
                    } else if (err != IO_EWOULDBLOCK) {
                        print_error("Recv failed");
                        break;
                    }
                } else {
                    recvbytes += r;
                    if (recvbytes >= bufsize) {
                        shutdown(connfd, IO_SHUT_RD);
                        printf("send and recv: %ld bytes - string: %s\n", (long)(recvbytes), buf);
                        break;
                    } else if (r == 0) {
                        shutdown(connfd, IO_SHUT_RD);
                        fprintf(stderr, "Recv truncate, send: %ld, recv: %ld bytes\n", (long)(sendbytes), (long)(recvbytes));
                        break;
                    }
                }
            }
        }
    } while (0);

    free(buf);
    if (connfd != INVALID_SOCKET) {
        close_socket(connfd);
    }
}

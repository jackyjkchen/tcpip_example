#include "io_server.h"

int server_socket_init(int protocol, int nonblock) {
    SOCKET listenfd;
    int reuse_addr = 1;
    struct sockaddr_in server_addr;

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

    if (protocol == TCP) {
        listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    } else if (protocol == UDP) {
        listenfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    } else {
        fprintf(stderr, "Unsupport protocol");
        return -1;
    }
    if (listenfd < 0) {
        print_error("Create socket failed");
        return -1;
    }

    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuse_addr, sizeof(reuse_addr)) != 0) {
        print_error("Set SO_REUSEADDR failed");
        return -1;
    }

    memset(&server_addr, 0x00, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERV_PORT);

    if (bind(listenfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) != 0) {
        close_socket(listenfd);
        print_error("Bind server_addr failed");
        return -1;
    }

    if (protocol == TCP) {
        if (listen(listenfd, LISTENQ) != 0) {
            close_socket(listenfd);
            print_error("Listen port failed");
            return -1;
        }
    }

    if (nonblock) {
#ifdef _WIN32
        u_long iMode = 1;

        if (ioctlsocket(listenfd, FIONBIO, &iMode) != NO_ERROR) {
#else
        if (fcntl(listenfd, F_SETFL, fcntl(listenfd, F_GETFL, 0) | O_NONBLOCK) < 0) {
#endif
            close_socket(listenfd);
            print_error("Set nonblock failed");
            return -1;
        }
    }
    return listenfd;
}

int reflect_server_callback(void *param) {
    int ret = 0;
    io_context_t *io_context = (io_context_t *)param;

#ifdef _WIN32
    SOCKET fd = (SOCKET)(io_context->fd);
#else
    SOCKET fd = (SOCKET)(long)(io_context->fd);
#endif
    io_context->sendagain = 0;
    while (1) {
        ssize_t n = 0;

        if (io_context->recvbytes == io_context->sendbytes) {
            n = recv(fd, io_context->buf + io_context->recvbytes, io_context->bufsize - io_context->recvbytes, 0);
            if (n == 0) {
                shutdown(fd, IO_SHUT_RD);
            }
        }
        if (n > 0 || (n == 0 && io_context->recvbytes > io_context->sendbytes)) {
            ssize_t w = 0;

            io_context->recvbytes += n;
            if (io_context->recvbytes - io_context->sendbytes > 0) {
                w = send(fd, io_context->buf + io_context->sendbytes,
                         io_context->recvbytes - io_context->sendbytes, MSG_NOSIGNAL);
            }
            if (w < 0) {
                int err = get_last_error();

                if (err == IO_EINTR) {
                    continue;
                } else if (err == IO_EWOULDBLOCK) {
                    io_context->sendagain = 1;
                }
                ret = -1;
                break;
            } else {
                io_context->sendbytes += w;
            }
        } else if (n < 0) {
            int err = get_last_error();

            if (err == IO_EINTR) {
                continue;
            }
            ret = -1;
            break;
        } else {
            if (io_context->sendbytes == io_context->recvbytes) {
                int err = get_last_error();

                if (err != IO_EWOULDBLOCK && err != IO_OK) {
                    ret = -1;
                }
                shutdown(fd, IO_SHUT_WR);
                break;
            }
        }
        if (io_context->recvbytes == io_context->bufsize && io_context->sendbytes == io_context->recvbytes) {
            io_context->recvbytes = 0;
            io_context->sendbytes = 0;
        }
    }
    return ret;
}

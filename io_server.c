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
    if (listenfd == INVALID_SOCKET) {
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
    server_addr.sin_port = htons(protocol == TCP ? TCP_SERV_PORT : UDP_SERV_PORT);

    if (bind(listenfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0) {
        print_error("Bind server_addr failed");
        close_socket(listenfd);
        return -1;
    }

    if (protocol == TCP) {
        if (listen(listenfd, LISTENQ) != 0) {
            print_error("Listen port failed");
            close_socket(listenfd);
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

void reflect_server_callback(void *param) {
    io_context_t *io_context = (io_context_t *)param;

#ifdef _WIN32
    SOCKET fd = (SOCKET)(io_context->fd);
#else
    SOCKET fd = (SOCKET)(long)(io_context->fd);
#endif
    io_context->sendagain = 0;
    while (1) {
        ssize_t r = 0;

        if (!io_context->recvdone && io_context->bufsize > io_context->recvbytes) {
            r = recv(fd, io_context->buf + io_context->recvbytes, io_context->bufsize - io_context->recvbytes, 0);
            if (r == 0 && io_context->bufsize > io_context->recvbytes) {
                shutdown(fd, IO_SHUT_RD);
                io_context->recvdone = 1;
            }
        }
        if (r > 0 || (r == 0 && io_context->recvbytes > io_context->sendbytes)) {
            ssize_t w = 0;

            io_context->recvbytes += r;
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
                break;
            } else {
                io_context->sendbytes += w;
                if (io_context->recvdone) {
                    if (io_context->recvbytes == io_context->sendbytes) {
                        shutdown(fd, IO_SHUT_WR);
                    }
                }
            }
        } else if (r < 0) {
            int err = get_last_error();

            if (err == IO_EINTR) {
                continue;
            }
            break;
        } else {
            set_last_error(IO_OK);
            break;
        }
        if (io_context->recvbytes == io_context->bufsize && io_context->sendbytes == io_context->recvbytes) {
            io_context->recvbytes = 0;
            io_context->sendbytes = 0;
        }
    }
}

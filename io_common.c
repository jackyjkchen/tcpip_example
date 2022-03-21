#include <sys/resource.h>
#include <sys/select.h>
#include <poll.h>
#include "io_common.h"

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

ssize_t nread(int fd, void *buf, size_t buf_size)
{
    size_t nleft = buf_size;
    ssize_t nread = 0;
    unsigned char *p = (unsigned char *)buf;

    while (nleft > 0){
        if ((nread = read(fd, p, nleft)) < 0) {
            if (errno == EINTR) {
                nread = 0;
            } else if (errno == EWOULDBLOCK) {
                return nleft - buf_size;
            } else {
                return -1;
            }
        } else if (nread == 0){
            break;
        }

        nleft -= nread;
        p += nread;
    }

    return buf_size - nleft;
}

ssize_t nwrite(int fd, const void *buf, size_t buf_size)
{
    size_t nleft = buf_size;
    ssize_t nwritten = 0;
    const unsigned char *p = (const unsigned char *)buf;

    while (nleft > 0) {
        if ((nwritten = write(fd, p, nleft)) <= 0) {
            if (nwritten < 0 && errno == EINTR) {
                nwritten = 0;
            } else if (nwritten < 0 && errno == EWOULDBLOCK) {
                return nleft - buf_size;
            } else {
                return -1;
            }
        }

        nleft -= nwritten;
        p += nwritten;
    }
    return buf_size - nleft;
}

int set_rlimit()
{
    struct rlimit rl;
    rl.rlim_cur = 65536;
    rl.rlim_max = 65536;
    return setrlimit(RLIMIT_NOFILE, &rl);
}

int reflect_server_callback(void *param)
{
    ssize_t n = 0;
    int ret = 0;
    int connfd = (int)(long)param;
    unsigned char buf[BUF_SIZE];
    while (1) {
        n = nread(connfd, buf, BUF_SIZE);
        if (n < 0 && errno != EWOULDBLOCK) {
            perror("Recv failed");
            close(connfd);
            ret = -1;
            break;
        } else {
            ssize_t w = nwrite(connfd, buf, n>=0?n:-n);
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

void reflect_client_callback(void *param)
{
    int connfd = 0;
    unsigned char buf[BUF_SIZE] = {0};
    const char *s = "hello, world\n";
    struct sockaddr_in *pserver_addr = (struct sockaddr_in *)param;
    do {
        int connfd = socket(PF_INET, SOCK_STREAM, 0);
        if (connfd < 0) {
            perror("Create socket failed");
            break;
        }

        if (connect(connfd, (const struct sockaddr *)pserver_addr, sizeof(struct sockaddr_in)) != 0) {
            perror("Connect failed");
            break;
        }

        if (nwrite(connfd, s, strlen(s)+1) < 0) {
            perror("Send failed");
        } else {
            shutdown(connfd, SHUT_WR);
            if (nread(connfd, buf, BUF_SIZE) < 0) {
                perror("Recv failed");
            } else {
                printf("%s", buf);
            }
        }
    } while(0);

    if (connfd > 0) {
        close(connfd);
    }
}

void poll_loop(int listenfd, server_callback svrcbk)
{
    int connfd, ready_num;
    int i = 0, maxi = 0;
    socklen_t client_addr_len;
    struct sockaddr_in client_addr;
    struct pollfd client[MAX_CONN];

    client[0].fd = listenfd;
    client[0].events = POLLRDNORM;
    for (i=1; i<MAX_CONN; ++i) {
        client[i].fd = -1;
    }

    for (;;) {
        ready_num = poll(client, maxi + 1, -1);
        if (ready_num < 0) {
            perror("Poll failed");
        }

        if (client[0].revents == POLLRDNORM) {
            client_addr_len = sizeof(client_addr);
            connfd = accept(listenfd, (struct sockaddr *)&client_addr, &client_addr_len);
            if (connfd < 0) {
                perror("Accept failed");
                continue;
            }

            if (fcntl(connfd, F_SETFL, fcntl(connfd, F_GETFL, 0) | O_NONBLOCK) < 0) {
                close(connfd);
                perror("Set nonblock failed");
                continue;
            }

            for (i=0; i<MAX_CONN; ++i) {
                if (client[i].fd < 0) {
                    client[i].fd = connfd;
                    break;
                }
            }

            if (i >= MAX_CONN) {
                close(connfd);
                puts("Too many clients.");
                continue;
            }

            client[i].events = POLLRDNORM;
            if (i > maxi) {
                maxi = i;
            }
            if (--ready_num <= 0) {
                continue;
            }
        }

        for (i=1; i<=maxi; ++i) {
            if ((connfd = client[i].fd) < 0) {
                continue;
            }
            if (client[i].revents & POLLRDNORM) {
                int ret = svrcbk((void*)(long)connfd);
                if (ret != -2) {
                    client[i].fd = -1;
                }
                if (--ready_num <= 0) {
                    break;
                }
            } else if (client[i].revents & POLLERR) {
                close(connfd);
                client[i].fd = -1;
                if (--ready_num <= 0) {
                    break;
                }
            }
        }
    }
}

void select_loop(int listenfd, server_callback svrcbk)
{
    int connfd, maxfd;
    int ready_num, client[MAX_CONN];
    int i = 0, maxi = -1;
    socklen_t client_addr_len;
    struct sockaddr_in client_addr;
    fd_set allset, rset;

    maxfd = listenfd;
    for (i=0; i<MAX_CONN; ++i) {
        client[i] = -1;
    }
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);

    for (;;) {
        rset = allset;
        ready_num = select(maxfd+1, &rset, NULL, NULL, NULL);
        if (ready_num < 0) {
            perror("Select failed");
        }

        if (FD_ISSET(listenfd, &rset)) {
            client_addr_len = sizeof(client_addr);
            connfd = accept(listenfd, (struct sockaddr *)&client_addr, &client_addr_len);
            if (connfd < 0) {
                perror("Accept failed");
                continue;
            }

            if (fcntl(connfd, F_SETFL, fcntl(connfd, F_GETFL, 0) | O_NONBLOCK) < 0) {
                close(connfd);
                perror("Set nonblock failed");
                continue;
            }

            for (i=0; i<MAX_CONN; ++i) {
                if (client[i] < 0) {
                    client[i] = connfd;
                    break;
                }
            }

            if (i >= MAX_CONN) {
                close(connfd);
                puts("Too many clients.");
                continue;
            }

            FD_SET(connfd, &allset);
            if (connfd > maxfd) {
                maxfd = connfd;
            }
            if (i > maxi) {
                maxi = i;
            }
            if (--ready_num <= 0) {
                continue;
            }
        }

        for (i=0; i<=maxi; ++i) {
            if ((connfd = client[i]) < 0) {
                continue;
            }
            if (FD_ISSET(connfd, &rset)) {
                int ret = svrcbk((void*)(long)connfd);
                if (ret != -2) {
                    FD_CLR(connfd, &allset);
                    client[i] = -1;
                }
                if (--ready_num <= 0) {
                    break;
                }
            }
        }
    }
}

void accept_loop(int listenfd, server_callback svrcbk)
{
    int connfd;
    socklen_t client_addr_len;
    struct sockaddr_in client_addr;

    for (;;) {
        client_addr_len = sizeof(client_addr);
        connfd = accept(listenfd, (struct sockaddr *)&client_addr, &client_addr_len);
        if (connfd < 0) {
            perror("Accept failed");
            continue;
        }
        svrcbk((void*)(long)connfd);
    }
}

#ifdef __cplusplus
}
#endif


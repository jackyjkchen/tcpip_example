#include <sys/resource.h>
#include "io_client.h"

#ifdef __cplusplus
extern "C" {
#endif

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

        if (writen(connfd, s, strlen(s)+1) < 0) {
            perror("Send failed");
        } else {
            shutdown(connfd, SHUT_WR);
            if (readn(connfd, buf, BUF_SIZE) < 0) {
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

#ifdef __cplusplus
}
#endif


#include <sys/time.h>
#ifndef _WIN32
#include <sys/resource.h>
#endif
#include "io_common.h"

#ifdef __cplusplus
extern "C" {
#endif

int close_socket(SOCKET fd) {
#ifdef _WIN32
    return closesocket(fd);
#else
    return close(fd);
#endif
}

int get_last_error() {
#ifdef _WIN32
    return WSAGetLastError();
#else
    return errno;
#endif
}

#ifndef _WIN32
int set_rlimit()
{
    struct rlimit rl;
    rl.rlim_cur = 65536;
    rl.rlim_max = 65536;
    return setrlimit(RLIMIT_NOFILE, &rl);
}
#endif

#ifdef __cplusplus
}
#endif


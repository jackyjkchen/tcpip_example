#ifndef _WIN32
#include <sys/resource.h>
#endif
#include "io_common.h"

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

void set_last_error(int err) {
#ifdef _WIN32
    WSASetLastError(err);
#else
    errno = err;
#endif
}

void print_error(const char *msg) {
#ifdef _WIN32
    char msgbuf[256] = { 0 };
    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                   NULL, WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), msgbuf, sizeof(msgbuf), NULL);
    fprintf(stderr, "%s: %s\n", msg, msgbuf);
#else
    perror(msg);
#endif
}

#ifndef _WIN32
int set_rlimit() {
    struct rlimit rl;

#ifdef __CYGWIN__
    rl.rlim_cur = 2048;
    rl.rlim_max = 2048;
#else
    rl.rlim_cur = 65536;
    rl.rlim_max = 65536;
#endif
    return setrlimit(RLIMIT_NOFILE, &rl);
}
#endif

#include "io_WSAEventSelect.h"

#ifdef __cplusplus
extern "C" {
#endif

static void free_event(int index, int *total, WSAEVENT *events) {
    int i = 0;
    WSACloseEvent(events[index]);
    for (i=index; i<*total; i++) {
        events[i] = events[i+1];
    }
    events[i] = WSA_INVALID_EVENT;
    (*total)--;
}

int WSAEventSelect_loop(SOCKET listenfd, server_callback svrcbk)
{
    SOCKET connfd;
    int i = 0, event_total = 0, ret = 0;
    WSAEVENT events[WSA_MAXIMUM_WAIT_EVENTS] = {0};

	for (i=0; i<WSA_MAXIMUM_WAIT_EVENTS; i++) {
        events[i] = WSA_INVALID_EVENT;
    }

    if ((events[event_total] = WSACreateEvent()) == WSA_INVALID_EVENT) {
        print_error("WSACreateEvent listenfd failed");
        return -1;
    }

    if (WSAEventSelect(listenfd, events[event_total], FD_ACCEPT | FD_CLOSE) == SOCKET_ERROR) {
        print_error("WSAEventSelect listenfd failed");
        return -1;
    }

    alloc_io_context(events[event_total]);
    get_io_context(events[event_total++])->fd = (void*)listenfd;
    for (;;) {
        WSANETWORKEVENTS network_event;
        int event_index = 0;
        WSAEVENT ev;
        io_context_t *io_context = NULL;
        if ((event_index = WSAWaitForMultipleEvents(event_total, events, FALSE, WSA_INFINITE, FALSE)) == WSA_WAIT_FAILED) {
            print_error("WSAWaitForMultipleEvents failed");
            ret = -1;
            break;
        }
        ev = events[event_index - WSA_WAIT_EVENT_0];
        io_context = get_io_context(ev);
        connfd = (SOCKET)(io_context->fd);
        if (WSAEnumNetworkEvents(connfd, ev, &network_event) == SOCKET_ERROR) {
            print_error("WSAEnumnetwork_event failed");
            ret = -1;
            break;
        }

        if (network_event.lNetworkEvents & FD_ACCEPT) {
            u_long iMode = 1;
            if (network_event.iErrorCode[FD_ACCEPT_BIT] != 0) {
                fprintf(stderr, "FD_ACCEPT failed with error %d\n", network_event.iErrorCode[FD_ACCEPT_BIT]);
                continue;
            }
            if ((connfd = accept(listenfd, NULL, NULL)) == INVALID_SOCKET) {
                print_error("accept failed");
                continue;
            }
            if (event_total >= WSA_MAXIMUM_WAIT_EVENTS) {
                fprintf(stderr, "Too many connections - closing socket...\n");
                close_socket(connfd);
                continue;
            }
            if (ioctlsocket(connfd, FIONBIO, &iMode) != NO_ERROR ) {
                print_error("Set nonblock failed");
                close_socket(connfd);
                ret = -1;
                break;
            }
            if ((events[event_total] = WSACreateEvent()) == WSA_INVALID_EVENT) {
                print_error("WSACreateEvent connfd failed");
                close_socket(connfd);
                ret = -1;
                break;
            }
            if (WSAEventSelect(connfd, events[event_total], FD_READ | FD_WRITE | FD_CLOSE) == SOCKET_ERROR) {
                print_error("WSAEventSelect connfd failed");
                close_socket(connfd);
                ret = -1;
                break;
            }
            alloc_io_context(events[event_total]);
            get_io_context(events[event_total++])->fd = (void*)connfd;
        } else if (network_event.lNetworkEvents & FD_READ || network_event.lNetworkEvents & FD_WRITE) {
            if (network_event.lNetworkEvents & FD_READ && network_event.iErrorCode[FD_READ_BIT] != 0) {
                fprintf(stderr, "FD_READ failed with error %d\n", network_event.iErrorCode[FD_READ_BIT]);
                continue;
            }
            if (network_event.lNetworkEvents & FD_WRITE && network_event.iErrorCode[FD_WRITE_BIT] != 0) {
                fprintf(stderr, "FD_WRITE failed with error %d\n", network_event.iErrorCode[FD_WRITE_BIT]);
                continue;
            }
            if (svrcbk(io_context) < 0 ) {
                if (get_last_error() != IO_EWOULDBLOCK) {
                    close_socket(connfd);
                    free_io_context(ev);
                    free_event(event_index - WSA_WAIT_EVENT_0, &event_total, events);
                } else if (io_context->sendagain) {
                    continue;
                }
            }
        }
        if (network_event.lNetworkEvents & FD_CLOSE) {
            if (network_event.iErrorCode[FD_CLOSE_BIT] != 0) {
                fprintf(stderr, "FD_CLOSE failed with error %d\n", network_event.iErrorCode[FD_CLOSE_BIT]);
                ret = -1;
                break;
            }
            close_socket(connfd);
            free_io_context(ev);
            free_event(event_index - WSA_WAIT_EVENT_0, &event_total, events);
        }
    }
    WSACloseEvent(events[0]);
    return ret;
}

#ifdef __cplusplus
}
#endif

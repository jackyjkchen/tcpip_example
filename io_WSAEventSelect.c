#include "io_WSAEventSelect.h"

#ifdef __cplusplus
extern "C" {
#endif

static void free_event(int index, int *total, WSAEVENT *pevents, SOCKET *psockets) {
    int i = 0;
    closesocket(psockets[index]);
    WSACloseEvent(pevents[index]);
    for (i=index; i<*total; i++) {
        pevents[i] = pevents[i + 1];
        psockets[i] = psockets[i + 1];
    }
    pevents[i] = WSA_INVALID_EVENT;
    psockets[i] = INVALID_SOCKET;
    (*total)--;
}

int WSAEventSelect_loop(SOCKET listenfd, server_callback svrcbk)
{
    SOCKET connfd;
    int i = 0, event_total = 0;
    socklen_t client_addr_len;
    struct sockaddr_in client_addr;
    WSAEVENT events[WSA_MAXIMUM_WAIT_EVENTS];
    SOCKET sockets[WSA_MAXIMUM_WAIT_EVENTS];

	for (i=0; i<WSA_MAXIMUM_WAIT_EVENTS; i++) {
        events[i] = WSA_INVALID_EVENT;
        sockets[i] = INVALID_SOCKET;
    }

    if ((events[event_total] = WSACreateEvent()) == WSA_INVALID_EVENT) {
        perror("WSACreateEvent listenfd failed");
        return -1;
    }

    if (WSAEventSelect(listenfd, events[event_total], FD_ACCEPT | FD_CLOSE) == SOCKET_ERROR) {
        perror("WSAEventSelect listenfd failed");
        return -1;
    }
    sockets[event_total++] = listenfd;

    for (;;) {
        WSANETWORKEVENTS network_event;
        int event_index = 0;
        if ((event_index = WSAWaitForMultipleEvents(event_total, events, FALSE, WSA_INFINITE, FALSE)) == WSA_WAIT_FAILED) {
            perror("WSAWaitForMultipleEvents failed");
            return -1;
        }
        if (WSAEnumNetworkEvents(sockets[event_index - WSA_WAIT_EVENT_0],
            events[event_index - WSA_WAIT_EVENT_0], &network_event) == SOCKET_ERROR) {
            perror("WSAEnumnetwork_event failed");
            return -1;
        }

        if (network_event.lNetworkEvents & FD_ACCEPT) {
            u_long iMode = 1;
            if (network_event.iErrorCode[FD_ACCEPT_BIT] != 0) {
                printf("FD_ACCEPT failed with error %d\n", network_event.iErrorCode[FD_ACCEPT_BIT]);
                continue;
            }
            if ((connfd = accept(sockets[event_index - WSA_WAIT_EVENT_0], NULL, NULL)) == INVALID_SOCKET) {
                perror("accept failed");
                continue;
            }
            if (event_total >= WSA_MAXIMUM_WAIT_EVENTS) {
                printf("Too many connections - closing socket...\n");
                closesocket(connfd);
                continue;
            }
            if (ioctlsocket(connfd, FIONBIO, &iMode) != NO_ERROR ) {
                perror("Set nonblock failed");
                closesocket(connfd);
                return -1;
            }
            if ((events[event_total] = WSACreateEvent()) == WSA_INVALID_EVENT) {
                perror("WSACreateEvent connfd failed");
                closesocket(connfd);
                return -1;
            }
            if (WSAEventSelect(connfd, events[event_total], FD_READ | FD_CLOSE) == SOCKET_ERROR) {
                perror("WSAEventSelect connfd failed");
                closesocket(connfd);
                return -1;
            }
            sockets[event_total++] = connfd;
        } else if (network_event.lNetworkEvents & FD_READ) {
            if (network_event.lNetworkEvents & FD_READ && network_event.iErrorCode[FD_READ_BIT] != 0) {
                printf("FD_READ failed with error %d\n", network_event.iErrorCode[FD_READ_BIT]);
                continue;
            }
            svrcbk((void*)sockets[event_index - WSA_WAIT_EVENT_0]);
            free_event(event_index - WSA_WAIT_EVENT_0, &event_total, events, sockets);
        } else if (network_event.lNetworkEvents & FD_CLOSE) {
            free_event(event_index - WSA_WAIT_EVENT_0, &event_total, events, sockets);
        }
    }
    WSACloseEvent(events[0]);
    return 0;
}

#ifdef __cplusplus
}
#endif

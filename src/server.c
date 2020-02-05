#include "util.h"
#include "server.h"

#include <stdio.h>

EXPORT CDTPServer cdtp_server(void (*on_recv      )(int, void *, void *),
                              void (*on_connect   )(int, void *, void *),
                              void (*on_disconnect)(int, void *, void *),
                              void *on_recv_arg, void *on_connect_arg, void *on_disconnect_arg,
                              int blocking, int event_blocking, int daemon)
{
#if defined(_WIN32) && !defined(CDTP_WINSOCK_INIT)
#define CDTP_WINSOCK_INIT
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        printf("Failed to initialize winsock: %d\n", WSAGetLastError());
        exit(EXIT_FAILURE);
    }
#endif
    CDTPServer server;
    server.on_recv           = on_recv;
    server.on_connect        = on_connect;
    server.on_disconnect     = on_disconnect;
    server.on_recv_arg       = on_recv_arg;
    server.on_connect_arg    = on_connect_arg;
    server.on_disconnect_arg = on_disconnect_arg;
    server.blocking          = blocking;
    server.event_blocking    = event_blocking;
    server.daemon            = daemon;
    server.serving           = CDTP_FALSE;

    // Initialize the socket
    int opt = 1;
#ifdef _WIN32
    if ((server.sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
    {
        printf("Failed to create socket: %d\n", WSAGetLastError());
        exit(EXIT_FAILURE);
    }
    if (setsockopt(server.sock, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) == SOCKET_ERROR)
    {
        printf("Failed to set socket option: %d\n", WSAGetLastError());
        exit(EXIT_FAILURE);
    }
#else
    if ((server.sock = socket(AF_INT, SOCK_STREAM, 0)) == 0)
    {
        printf("Failed to create socket\n");
        exit(EXIT_FAILURE);
    }
    if (setsockopt(server.sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        printf("Failed to set socket option\n");
        exit(EXIT_FAILURE);
    }
#endif
    return server;
}

EXPORT CDTPServer cdtp_server_default(void (*on_recv      )(int, void *, void *),
                                      void (*on_connect   )(int, void *, void *),
                                      void (*on_disconnect)(int, void *, void *),
                                      void *on_recv_arg, void *on_connect_arg, void *on_disconnect_arg)
{
    return cdtp_server(on_recv, on_connect, on_disconnect,
                       on_recv_arg, on_connect_arg, on_disconnect_arg,
                       CDTP_FALSE, CDTP_FALSE, CDTP_TRUE);
}

EXPORT int cdtp_start(CDTPServer *server, const char *host, const int port)
{
    // Make sure the server is not already serving
    if (server->serving == CDTP_TRUE)
        return CDTP_SERVER_ALREADY_SERVING;
    server->serving = CDTP_TRUE;

    // Set the server address
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    if (host == INADDR_ANY)
        address.sin_addr.s_addr = host;
    else
        address.sin_addr.s_addr = inet_addr(host);
    address.sin_port = htons(port);
    
    // Bind the address to the server
    if (bind(server->sock, (struct sockaddr *)&address, sizeof(address)) < 0)
        return CDTP_SERVER_BIND_FAILED;

    // Listen for connections
    if (listen(server->sock, CDTP_LISTEN_BACKLOG) < 0)
        return CDTP_SERVER_LISTEN_FAILED;

    // Serve
    if (server->blocking)
    {
        cdtp_serve(server);
    }
    else
    {
        // TODO: call `cdtp_serve` using thread
    }
    
    return CDTP_SERVER_SUCCESS;
}

EXPORT int cdtp_start_port(CDTPServer *server, const int port)
{
    return cdtp_start(server, INADDR_ANY, port);
}

EXPORT int cdtp_start_default(CDTPServer *server)
{
    return cdtp_start(server, INADDR_ANY, 0);
}

void cdtp_serve(CDTPServer *server)
{

}

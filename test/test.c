#include "../bin/include/cdtp.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

char *voidp_to_str(void *data, size_t data_size)
{
    char *data_str = (char *)data;
    char *message = malloc((data_size + 1) * sizeof(char));
    for (int i = 0; i < data_size; i++)
        message[i] = data_str[i];
    message[data_size] = '\0';
    return message;
}

void server_on_recv(int client_id, void *data, size_t data_size, void *arg)
{
    char *message = voidp_to_str(data, data_size);
    printf("Received data from client #%d: %s (size %ld)\n", client_id, message, data_size);
    free(data);
    free(message);
}

void server_on_connect(int client_id, void *arg)
{
    printf("Client #%d connected\n", client_id);
}

void server_on_disconnect(int client_id, void *arg)
{
    printf("Client #%d disconnected\n", client_id);
}

void client_on_recv(void *data, size_t data_size, void *arg)
{
    char *message = voidp_to_str(data, data_size);
    printf("Received data from server: %s (size %ld)\n", message, data_size);
    free(data);
    free(message);
}

void client_on_disconnected(void *arg)
{
    printf("Unexpectedly disconnected from server\n");
}

void on_err(int cdtp_err, int underlying_err, void *arg)
{
    printf("CDTP error:       %d\n", cdtp_err);
    printf("Underlying error: %d\n", underlying_err);
    exit(EXIT_FAILURE);
}

void check_err(void)
{
    if (cdtp_error())
        on_err(cdtp_get_error(), cdtp_get_underlying_error(), NULL);
}

int main(int argc, char **argv)
{
    double wait_time = 0.1;

    printf("Running tests...\n");

    // Register error event
    cdtp_on_error(on_err, NULL);

    // Server initialization
    CDTPServer *server = cdtp_server(16,
                                     server_on_recv, server_on_connect, server_on_disconnect,
                                     NULL, NULL, NULL,
                                     CDTP_FALSE, CDTP_FALSE);

    // Server start
    char *host = "127.0.0.1";
    cdtp_server_start_default_port(server, host);

    // Get IP address and port
    char *ip_address = cdtp_server_host(server);
    int port = cdtp_server_port(server);
    printf("IP address: %s\n", ip_address);
    printf("Port:       %d\n", port);

    // Unregister error event
    cdtp_on_error_clear();

    // Test that the client does not exist
    cdtp_server_remove_client(server, 0);
    assert(cdtp_get_error() == CDTP_CLIENT_DOES_NOT_EXIST);

    // Register error event
    cdtp_on_error(on_err, NULL);

    cdtp_sleep(wait_time);

    // Client initialization
    CDTPClient *client = cdtp_client(client_on_recv, client_on_disconnected,
                                     NULL, NULL,
                                     CDTP_FALSE, CDTP_FALSE);

    // Client connect
    cdtp_client_connect_default_port(client, ip_address);
    free(ip_address);

    // Get IP address and port
    char *client_ip_address = cdtp_client_host(client);
    int client_port = cdtp_client_port(client);
    printf("IP address: %s\n", client_ip_address);
    printf("Port:       %d\n", client_port);
    free(client_ip_address);

    cdtp_sleep(wait_time);

    // Client send
    char *client_message = "Hello, server.";
    cdtp_client_send(client, client_message, strlen(client_message));

    cdtp_sleep(wait_time);

    // Server send
    char *server_message = "Hello, client #0.";
    cdtp_server_send(server, 0, server_message, strlen(server_message));

    cdtp_sleep(wait_time);

    // Client disconnect
    cdtp_client_disconnect(client);

    cdtp_sleep(wait_time);

    // Server stop
    cdtp_server_stop(server);

    // Unregister error event
    cdtp_on_error_clear();

    // Test that server cannot be restarted
    cdtp_server_start_default_port(server, host);
    assert(cdtp_get_error() == CDTP_SERVER_CANNOT_RESTART);

    printf("Successfully passed all tests\n");
    return 0;
}

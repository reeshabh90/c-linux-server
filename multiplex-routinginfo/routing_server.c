#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include "routing_table.h" // change to "routing_table.c" while debugging in VS code

#define BUFFER_SIZE 260
#define MAX_CLIENT_SUPPORTED 32
#define PORT 8080

/// @brief An integer array to contain file descriptors to be monitored by select()
int monitor_fd_set[MAX_CLIENT_SUPPORTED];

/// @brief An integer array to contain computation results for each client
int client_result[MAX_CLIENT_SUPPORTED] = {0};

/// @brief Function to send routing table to a client
/// @param client_socket
void send_routing_table_to_client(int client_socket)
{
    char buffer[1024] = {0};
    int table_size = get_routing_table_size();
    RouteEntry *table = get_routing_table();

    // Serialize routing table into a buffer
    for (int i = 0; i < table_size; i++)
    {
        snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer),
                 "Destination: %s, Mask: %s, Gateway: %s, OIF: %s\n",
                 table[i].destination, table[i].mask,
                 table[i].gateway, table[i].oif);
    }
    printf("Sending routing entry to Client with fd %d\n", client_socket);
    // Send buffer to the connected client
    send(client_socket, buffer, strlen(buffer), 0);
}

/// @brief Send routing table change information to clients
/// @param client_sockets
/// @param num_clients
void send_routing_table_to_clients(int client_sockets[], int num_clients, int connection_socket)
{
    char buffer[1024] = {0};
    int table_size = get_routing_table_size();
    RouteEntry *table = get_routing_table();

    // Serialize routing table into a buffer
    for (int i = 0; i < table_size; i++)
    {
        snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer),
                 "Destination: %s, Mask: %s, Gateway: %s, OIF: %s\n",
                 table[i].destination, table[i].mask,
                 table[i].gateway, table[i].oif);
    }

    // Send buffer to all connected clients
    for (int i = 0; i < num_clients; i++)
    {
        // Skip invalid or stdin file descriptor (fd 0)
        if (client_sockets[i] > 0 && client_sockets[i] != connection_socket)
        {
            printf("Sending routing table to client with fd %d\n", client_sockets[i]);

            // Attempt to send the data and handle any potential errors
            ssize_t sent_bytes = send(client_sockets[i], buffer, strlen(buffer), 0);
            if (sent_bytes == -1)
            {
                perror("send");
            }
        }
    }
}

/** @brief
 * Initialize all elements of Monitor FD Set to -1.
 */
static void init_monitor_fd_set()
{
    for (int i = 0; i < MAX_CLIENT_SUPPORTED; i++)
    {
        monitor_fd_set[i] = -1;
    }
}

/**
 * @brief
 * Add file descriptors corresponding to clients in Monitor FD Set
 */
static void add_monitor_fd_set(int socket_fd)
{
    for (int i = 0; i < MAX_CLIENT_SUPPORTED; i++)
    {
        if (monitor_fd_set[i] != -1)
            continue;

        monitor_fd_set[i] = socket_fd;
        break;
    }
}

/**
 * @brief
 * Remove File Descriptor of a corresponding client from Monitor FD set
 */
static void remove_monitor_fd_set(int socket_fd)
{
    for (int i = 0; i < MAX_CLIENT_SUPPORTED; i++)
    {
        if (monitor_fd_set[i] != socket_fd)
            continue;

        monitor_fd_set[i] = -1;
        break;
    }
}

/// @brief Reset the FD set data structure with file descriptors present
/// in monitored fd set.
/// @param fd_set_ptr
static void refresh_fd_set(fd_set *fd_set_ptr)
{
    FD_ZERO(fd_set_ptr);
    for (int i = 0; i < MAX_CLIENT_SUPPORTED; i++)
    {
        if (monitor_fd_set[i] != -1)
            FD_SET(monitor_fd_set[i], fd_set_ptr);
    }
}

/// @brief Get the numerical max value among all FDs which server is monitoring
/// @return returns MAX integer value
static int get_max_fd()
{

    int i = 0;
    int max = -1;
    for (; i < MAX_CLIENT_SUPPORTED; i++)
    {
        if (monitor_fd_set[i] > max)
            max = monitor_fd_set[i];
    }
    return max;
}

/// @brief Function to set a socket to non-blocking mode
void set_socket_non_blocking(int socket_fd)
{
    int flags = fcntl(socket_fd, F_GETFL, 0);
    if (flags == -1)
    {
        perror("fcntl get failed");
        exit(EXIT_FAILURE);
    }
    if (fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        perror("fcntl set non-blocking failed");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[])
{
    struct sockaddr_in server_addr;
    int ret;
    int connection_socket;
    int data_socket;
    int result;
    int data;
    char buffer[BUFFER_SIZE];
    fd_set readfds;
    int comm_socket_fd, i;
    init_monitor_fd_set();
    add_monitor_fd_set(0);
    init_routing_table();
    RouteEntry entry1 = {"192.168.1.0", "255.255.255.0", "192.168.1.1", "eth0"};
    add_route(entry1);
    // unlink(SOCKET_NAME);

    /* Create Master socket. */
    connection_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (connection_socket == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    set_socket_non_blocking(connection_socket);
    printf("Master socket created\n");
    // Initialize
    memset(&server_addr, 0, sizeof(struct sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Socket Binding
    ret = bind(connection_socket, (const struct sockaddr *)&server_addr,
               sizeof(struct sockaddr_in));

    if (ret == -1)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    printf("bind() call succeed\n");
    // Listen for incoming connection
    ret = listen(connection_socket, 20);
    if (ret == -1)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    printf("Master COnnection Socket: %d\n", connection_socket);
    add_monitor_fd_set(connection_socket);
    // run an infinite loop
    for (;;)
    {
        refresh_fd_set(&readfds);

        printf("Waiting on select() sys call\n");
        int activity = select(get_max_fd() + 1, &readfds, NULL, NULL, NULL);

        if (activity < 0)
        {
            perror("select");
            exit(EXIT_FAILURE);
        }

        printf("Check for new connection requests\n");

        if (FD_ISSET(connection_socket, &readfds))
        {
            printf("New connection received, accepting the connection: \n");

            data_socket = accept(connection_socket, NULL, NULL);

            if (data_socket == -1)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }
            set_socket_non_blocking(data_socket);
            printf("Connection accepted from client: %d\n", data_socket);

            add_monitor_fd_set(data_socket);
            // Send the current routing table to the new client
            send_routing_table_to_client(data_socket);
        }
        else if (FD_ISSET(0, &readfds))
        { // Checking if there is input on console.
            memset(buffer, 0, BUFFER_SIZE);
            ret = read(0, buffer, BUFFER_SIZE);
            printf("Input read from console : %s\n", buffer);
        }
        else
        {
            // Handle incoming data from clients
            for (int i = 0; i < MAX_CLIENT_SUPPORTED; i++)
            {
                if (monitor_fd_set[i] != -1 && FD_ISSET(monitor_fd_set[i], &readfds))
                {
                    ret = read(monitor_fd_set[i], buffer, BUFFER_SIZE);
                    if (ret <= 0)
                    {
                        // Handle exceptions
                        if (ret == 0)
                        {
                            // Connection closed by client
                            printf("Client disconnected\n");
                        }
                        else
                        {
                            perror("recv");
                        }
                        close(monitor_fd_set[i]);
                        remove_monitor_fd_set(monitor_fd_set[i]);
                    }
                    else
                    {
                        /* Handle data received in buffer */
                        RouteEntry new_entry;
                        sscanf(buffer, "%s %s %s %s", new_entry.destination, new_entry.mask, new_entry.gateway, new_entry.oif);

                        // Add the route entry
                        if (add_route(new_entry))
                        {
                            printf("Route added by client: %s %s %s %s\n",
                                   new_entry.destination, new_entry.mask,
                                   new_entry.gateway, new_entry.oif);

                            // Notify all clients about the updated routing table
                            send_routing_table_to_clients(monitor_fd_set, MAX_CLIENT_SUPPORTED, connection_socket);
                        }
                        else
                        {
                            printf("Failed to add route from client\n");
                        }
                    }
                }
            }
        }
    }
    close(connection_socket);
    remove_monitor_fd_set(connection_socket);
    printf("connection closed..\n");
    // Release resources after termination
    // unlink(SOCKET_NAME);
    exit(EXIT_SUCCESS);
}
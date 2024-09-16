#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/select.h>
#include "routing_table.h"

#define SOCKET_NAME "/tmp/DemoSocket"
#define BUFFER_SIZE 128

#define MAX_CLIENT_SUPPORTED 32

/// @brief An integer array to contain file descriptors to be monitored by select()
int monitor_fd_set[MAX_CLIENT_SUPPORTED];

/// @brief An integer array to contain computation results for each client
int client_result[MAX_CLIENT_SUPPORTED] = {0};

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
        if(monitor_fd_set[i] != -1)
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
        if(monitor_fd_set[i] != socket_fd)
        continue;

        monitor_fd_set[i] = -1;
        break;
    }
}

/// @brief Reset the FD set data structure with file descriptors present
/// in monitored fd set.
/// @param fd_set_ptr 
static void refresh_fd_set(fd_set *fd_set_ptr){
    FD_ZERO(fd_set_ptr);
    for (int i = 0; i < MAX_CLIENT_SUPPORTED; i++)
    {
        if(monitor_fd_set[i] != -1)
        FD_SET(monitor_fd_set[i], fd_set_ptr);
    }
}

/// @brief Get the numerical max value among all FDs which server is monitoring
/// @return returns MAX integer value
static int get_max_fd(){

    int i = 0;
    int max = -1;
    for(; i < MAX_CLIENT_SUPPORTED; i++){
        if(monitor_fd_set[i] > max)
            max = monitor_fd_set[i];
    }
    return max;
}

int main(int argc, char *argv[])
{
    struct sockaddr_un name;
    int table_size = 0;
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

    unlink(SOCKET_NAME);

    /* Create Master socket. */
    connection_socket = socket(AF_UNIX, SOCK_STREAM, 0);

    if (connection_socket == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    printf("Master socket created\n");

    /*initialize*/
    memset(&name, 0, sizeof(struct sockaddr_un));

    name.sun_family = AF_UNIX;
    strncpy(name.sun_path, SOCKET_NAME, sizeof(name.sun_path) - 1);

    ret = bind(connection_socket, (const struct sockaddr *)&name,
               sizeof(struct sockaddr_un));

    if (ret == -1)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    printf("bind() call succeed\n");

    ret = listen(connection_socket, 20);
    if (ret == -1)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    add_monitor_fd_set(connection_socket);

    for (;;)
    {
        refresh_fd_set(&readfds);
        printf("Waiting on select() sys call\n");

        select(get_max_fd() + 1, &readfds, NULL, NULL, NULL);
        /* Wait for incoming connection. */
        printf("Waiting on accept() sys call\n");

        if(FD_ISSET(connection_socket, &readfds)){

            /*Data arrives on Master socket only when new client connects with the server */
            printf("New connection recieved recvd, accept the connection\n");

            data_socket = accept(connection_socket, NULL, NULL);

            if (data_socket == -1) {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            printf("Connection accepted from client\n");

            add_monitor_fd_set(data_socket);
        }
         else if(FD_ISSET(0, &readfds)){
            memset(buffer, 0, BUFFER_SIZE);
            ret = read(0, buffer, BUFFER_SIZE);
            printf("Input read from console : %s\n", buffer);
        }
        else /* Data srrives on some other client FD*/
        {
            /*Find the client which has send us the data request*/
            i = 0, comm_socket_fd = -1;
            for(; i < MAX_CLIENT_SUPPORTED; i++){

                if(monitor_fd_set[i] != -1 && FD_ISSET(monitor_fd_set[i], &readfds)){
                    comm_socket_fd = monitor_fd_set[i];

                    /*Prepare the buffer to recv the data*/
                    memset(buffer, 0, BUFFER_SIZE);

                    /* Wait for next data packet. */
                    /*Server is blocked here. Waiting for the data to arrive from client
                     * 'read' is a blocking system call*/
                    printf("Waiting for data from the client\n");
                    ret = read(comm_socket_fd, buffer, BUFFER_SIZE);

                    if (ret == -1) {
                        perror("read");
                        exit(EXIT_FAILURE);
                    }

                    /* Add received summand. */
                    memcpy(&data, buffer, sizeof(int));
                    if(data == 0) {
                        /* Send result. */
                        memset(buffer, 0, BUFFER_SIZE);
                        sprintf(buffer, "Result = %d", client_result[i]);

                        printf("sending final result back to client\n");
                        ret = write(comm_socket_fd, buffer, BUFFER_SIZE);
                        if (ret == -1) {
                            perror("write");
                            exit(EXIT_FAILURE);
                        }

                        /* Close socket. */
                        close(comm_socket_fd);
                        client_result[i] = 0; 
                        remove_monitor_fd_set(comm_socket_fd);
                        continue; /*go to select() and block*/
                    }
                    client_result[i] += data;
                }
            }
        }
    }

    /*close the master socket*/
    close(connection_socket);
    remove_monitor_fd_set(connection_socket);
    printf("connection closed..\n");

    /* Server should release resources before getting terminated.
     * Unlink the socket. */

    unlink(SOCKET_NAME);
    exit(EXIT_SUCCESS);
}
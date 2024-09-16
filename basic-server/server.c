#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_NAME "/tmp/FirstSocket" // Socket Name
#define BUFFER_SIZE 128 

int main(int argc, char const *argv[])
{
    struct sockaddr_un name;

    #if 0
        struct sockaddr_un {
            sa_family_t sun_family; /* AF_UNIX */
            char sun_path[108];  /*pathname*/
        }
    #endif

    int ret, connection_socket, data_socket, result, data;
    char buffer[BUFFER_SIZE];

    // IN case of program failure, socket should be removed.
    unlink(SOCKET_NAME);

    // Master Socket creation
    connection_socket = socket(AF_UNIX, SOCK_STREAM, 0);

    if (connection_socket == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    
    printf("Master Socket created!\n");

    // Initialize for binding
    memset(&name, 0, sizeof(struct sockaddr_un));

    name.sun_family = AF_UNIX;
    strncpy(name.sun_path, SOCKET_NAME, sizeof(name.sun_path)-1);

    // Binding socket to socket name.
    /**
     * The bind() function expects a pointer to a struct sockaddr, but here we are working with struct sockaddr_un.
     * Since bind() expects the generic struct sockaddr * type,
     * we need to cast our struct sockaddr_un * to struct sockaddr *. 
     * The 'const' qualifier ensures that bind() won't modify the contents of the name structure. Done for safety.
     * The socket address data (name) remains unchanged during the bind() operation.
     * This is a standard practice in socket programming.
     * Code Meaning: Treat this pointer (&name, which points to sockaddr_un) as if it points to a sockaddr.
     */
    ret = bind(connection_socket, (const struct sockaddr *) &name, sizeof(struct sockaddr_un));

    if (ret == -1)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    
    printf("Bind() call is successful!\n");

    // Listen system call
    ret = listen(connection_socket, 20);
    if(ret == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    printf("Connection accepted from client\n");
    result =0;
     for (;;) {

        /* Wait for incoming connection. */
        printf("Waiting on accept() sys call\n");

        data_socket = accept(connection_socket, NULL, NULL);
        // accept sys call is a blocking call. Will block code exec until connection request are received.

        if (data_socket == -1) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        
        printf("Connection accepted from client\n");

        result = 0;
        for(;;) {

            /*Prepare the buffer to recv the data*/
            memset(buffer, 0, BUFFER_SIZE);

            /* Wait for next data packet. */
            /* Server is blocked here. Waiting for the data to arrive from client
             * 'read' is a blocking system call.
             */
            printf("Waiting for data from the client\n");
            ret = read(data_socket, buffer, BUFFER_SIZE);

            if (ret == -1) {
                perror("read");
                exit(EXIT_FAILURE);
            }

            /* Add received summand. */
            memcpy(&data, buffer, sizeof(int));
            if(data == 0) break;
            result += data;
        }

        // Sending result.
        memset(buffer, 0, BUFFER_SIZE);
        sprintf(buffer, "Result = %d", result);

        printf("sending final result back to client\n");
        ret = write(data_socket, buffer, BUFFER_SIZE);
        if (ret == -1) {
            perror("write");
            exit(EXIT_FAILURE);
        }

        // Closing socket.
        close(data_socket);
    }

    // closing the master socket
    close(connection_socket);
    printf("connection closed..\n");

    /* Server should release resources before getting terminated.
     * Unlink the socket.
     */

    unlink(SOCKET_NAME);
    exit(EXIT_SUCCESS);

}



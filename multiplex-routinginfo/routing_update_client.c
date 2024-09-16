#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 260

int main(int argc, char *argv[])
{
    struct sockaddr_in server_addr;
    int ret;
    int data_socket;
    char buffer[BUFFER_SIZE];
    char destination[64], mask[64], gateway[64], oif[64];
    // Check if server IP is provided
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <server_ip>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    // Create data socket
    data_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (data_socket == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Clear the sockaddr_un structure
    memset(&server_addr, 0, sizeof(struct sockaddr_in));

    // Set up the server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(argv[1]); // Loopback address for local machine 127.0.0.1
    server_addr.sin_port = htons(PORT);

    ret = connect(data_socket, (const struct sockaddr *)&server_addr, sizeof(struct sockaddr_in));
    if (ret == -1)
    {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    // Prepare route entry data
    // Receive and print data from server
    while (1)
    {
        printf("\nEnter new route (format: destination mask gateway oif) or type 'exit' to quit:\n");

        // Take input from the user
        scanf("%s", destination);
        if (strcmp(destination, "exit") == 0)
        {
            break;
        }
        // reading input
        scanf("%s %s %s", mask, gateway, oif);
        snprintf(buffer, sizeof(buffer), "%s %s %s %s", destination, mask, gateway, oif);

        // Send route entry to the server
        if (send(data_socket, buffer, strlen(buffer), 0) == -1)
        {
            perror("send");
        }
        else
        {
            printf("Route entry sent to server: %s %s %s %s\n", destination, mask, gateway, oif);
        }
        
        memset(buffer, 0, BUFFER_SIZE);
        ret = read(data_socket, buffer, BUFFER_SIZE - 1);
        if (ret == -1)
        {
            perror("read");
            break;
        }
        if (ret == 0)
        {
            printf("Server closed the connection.\n");
            break;
        }
        buffer[ret] = '\0'; // Ensure null-termination
        printf("Received: %s\n", buffer);
    }

    // Close socket
    close(data_socket);
    printf("Client disconnected.\n");
    return EXIT_SUCCESS;
}

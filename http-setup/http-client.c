#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>

#define PORT 8080
#define BUFFER_SIZE 1024

// Function to set the socket to non-blocking mode
void set_non_blocking(int socket_fd)
{
    fcntl(socket_fd, F_SETFL, O_NONBLOCK);
}

void send_request(int socket_fd, const char *method, const char *path, const char *body, struct sockaddr_in *server_addr)
{
    char request[BUFFER_SIZE];
    int request_len;
    char ip_str[INET_ADDRSTRLEN];
    // Convert the binary IP address to a human-readable string
    inet_ntop(AF_INET, &(server_addr->sin_addr), ip_str, INET_ADDRSTRLEN);

    if (strcmp(method, "GET") == 0)
    {
        request_len = snprintf(request, BUFFER_SIZE,
                               "GET %s HTTP/1.1\r\n"
                               "Host: %s:%d\r\n"
                               "Connection: close\r\n"
                               "\r\n",
                               path, ip_str, PORT);
    }
    else if (strcmp(method, "POST") == 0)
    {
        request_len = snprintf(request, BUFFER_SIZE,
                               "POST %s HTTP/1.1\r\n"
                               "Host: %s:%d\r\n"
                               "Connection: close\r\n"
                               "Content-Type: application/x-www-form-urlencoded\r\n"
                               "Content-Length: %ld\r\n"
                               "\r\n"
                               "%s",
                               path, ip_str, PORT, strlen(body), body);
    }
    else
    {
        fprintf(stderr, "Unsupported method: %s\n", method);
        return;
    }

    if (send(socket_fd, request, request_len, 0) != request_len)
    {
        perror("Failed to send request");
        return;
    }

    printf("Sent %s request to %s%s\n", method, ip_str, path);
}

int main(int argc, char *argv[])
{
    struct sockaddr_in server_addr;
    int ret;
    int data_socket;
    char buffer[BUFFER_SIZE];
    // Check if server IP is provided
    if (argc < 2)
    {
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

    send_request(data_socket, "GET", "/", NULL, &server_addr);
    memset(buffer, 0, BUFFER_SIZE);
    ret = read(data_socket, buffer, BUFFER_SIZE - 1);
    if (ret == -1)
    {
        perror("read");
        return -1;
    }
    if (ret == 0)
    {
        printf("Server closed the connection.\n");
        return 0;
    }
    buffer[ret] = '\0'; // Ensure null-termination
    printf("Received: %s\n", buffer);
    // Close socket
    close(data_socket);
    printf("Client disconnected.\n");
    return EXIT_SUCCESS;
}

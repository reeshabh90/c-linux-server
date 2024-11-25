#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define CONTROL_PORT 21
#define DATA_PORT 2021
#define SERVER_IP "127.0.0.1"
#define BUFFER_SIZE 1024

int main() {
    int control_socket, data_socket;
    struct sockaddr_in server_addr, data_addr;
    char buffer[BUFFER_SIZE];
    int p1 = DATA_PORT / 256;
    int p2 = DATA_PORT % 256;

    // Create control connection
    control_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (control_socket < 0) {
        perror("Control socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(CONTROL_PORT);

    // Connect to server control port
    if (connect(control_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Control connection failed");
        close(control_socket);
        exit(EXIT_FAILURE);
    }

    printf("Connected to FTP server.\n");

    // Step 1: Send Username
    snprintf(buffer, BUFFER_SIZE, "ftpClient\n");
    write(control_socket, buffer, strlen(buffer));
    read(control_socket, buffer, BUFFER_SIZE);
    printf("Server: %s", buffer);

    // Step 2: Send Password
    snprintf(buffer, BUFFER_SIZE, "ftpCLPass\n");
    write(control_socket, buffer, strlen(buffer));
    read(control_socket, buffer, BUFFER_SIZE);
    printf("Server: %s", buffer);

    // Step 3: Send PORT Command
    snprintf(buffer, BUFFER_SIZE, "PORT 127,0,0,1,%d,%d\n", p1, p2);
    write(control_socket, buffer, strlen(buffer));
    printf("Sent: %s", buffer);

    // Step 4: Send LIST Command
    snprintf(buffer, BUFFER_SIZE, "LIST\n");
    write(control_socket, buffer, strlen(buffer));
    read(control_socket, buffer, BUFFER_SIZE);
    printf("Server: %s", buffer);

    // Accept server connection on data port
    data_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (data_socket < 0) {
        perror("Data socket creation failed");
        close(control_socket);
        exit(EXIT_FAILURE);
    }

    memset(&data_addr, 0, sizeof(data_addr));
    data_addr.sin_family = AF_INET;
    data_addr.sin_addr.s_addr = INADDR_ANY;
    data_addr.sin_port = htons(DATA_PORT);

    if (bind(data_socket, (struct sockaddr *)&data_addr, sizeof(data_addr)) < 0) {
        perror("Data socket bind failed");
        close(control_socket);
        close(data_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(data_socket, 1) < 0) {
        perror("Data socket listen failed");
        close(control_socket);
        close(data_socket);
        exit(EXIT_FAILURE);
    }

    int server_data_socket = accept(data_socket, NULL, NULL);
    if (server_data_socket < 0) {
        perror("Data connection accept failed");
        close(control_socket);
        close(data_socket);
        exit(EXIT_FAILURE);
    }

    read(server_data_socket, buffer, BUFFER_SIZE);
    printf("Received Directory Listing:\n%s", buffer);

    close(server_data_socket);
    close(data_socket);
    close(control_socket);

    return 0;
}

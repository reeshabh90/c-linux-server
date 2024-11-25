
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <dirent.h>

#define BUFFER_SIZE 1024
#define CONTROL_PORT 2121
#define DATA_PORT 2021
#define USERNAME "ftpClient"
#define PASSWORD "ftpCLPass"
#define CLIENT_HOME_DIR "/home/ftpuser" // User's home directory

void *handle_client(void *arg);
void list_directory(const char *dir_path, int data_socket);

int main(int argc, char const *argv[])
{
    int server_fd, ftp_control_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    // Create control socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        perror("socket not created.");
        exit(EXIT_FAILURE);
    }

    // Prepare the sockaddr_in structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(CONTROL_PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, SOMAXCONN) == -1)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    for (;;)
    {
        /* Wait for incoming connection. */
        printf("Waiting on accept() sys call\n");

        ftp_control_socket = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        // accept sys call is a blocking call. Will block code exec until connection request are received.

        if (ftp_control_socket == -1)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        printf("Connection accepted from client :  %s\n", inet_ntoa(client_addr.sin_addr));

        // Handle each client in a new thread
        pthread_t client_thread;
        int *new_sock = malloc(sizeof(int));
        *new_sock = ftp_control_socket;
        // pthread_create requires the argument to be a void *
        if (pthread_create(&client_thread, NULL, handle_client, (void *)new_sock) < 0)
        {
            perror("Could not create thread");
            free(new_sock);
            close(ftp_control_socket);
        }
        pthread_detach(client_thread);
    }

    close(server_fd);
    return 0;
}

void *handle_client(void *arg)
{
    // casting the recieved argument back to integer
    int ftp_control_socket = *(int *)arg;
    free(arg);
    char buffer[BUFFER_SIZE];
    int ftp_data_socket;
    struct sockaddr_in client_data_addr;

    // Step 1: Receive Username
    memset(buffer, 0, BUFFER_SIZE);
    read(ftp_control_socket, buffer, BUFFER_SIZE);
    printf("Received Username: %s", buffer);
    if (strncmp(buffer, USERNAME, strlen(USERNAME)) != 0)
    {
        const char *response = "530 Invalid username.\n";
        write(ftp_control_socket, response, strlen(response));
        close(ftp_control_socket);
        return NULL;
    }
    const char *username_ok = "331 Username OK, need password.\n";
    write(ftp_control_socket, username_ok, strlen(username_ok));

    // Step 2: Receive Password
    memset(buffer, 0, BUFFER_SIZE);
    read(ftp_control_socket, buffer, BUFFER_SIZE);
    printf("Received Password: %s", buffer);
    if (strncmp(buffer, PASSWORD, strlen(PASSWORD)) != 0)
    {
        const char *response = "530 Login incorrect.\n";
        write(ftp_control_socket, response, strlen(response));
        close(ftp_control_socket);
        return NULL;
    }
    const char *login_ok = "230 Login successful.\n";
    write(ftp_control_socket, login_ok, strlen(login_ok));

    // Step 3: Receive PORT Command
    memset(buffer, 0, BUFFER_SIZE);
    read(ftp_control_socket, buffer, BUFFER_SIZE);
    printf("Received: %s\n", buffer);

    if (strncmp(buffer, "PORT", 4) == 0)
    {
        int p1, p2;
        char ip[16];
        // sample: "PORT 127,0,0,1,192,168\n"
        // The first 5 characters "PORT " are skipped
        // "%[^,]": This will read "127.0.0.1" (up to the first comma) into the ip variable.
        // "%d": This will read 192 into the p1 variable.
        // "%d": This will read 168 into the p2 variable.
        sscanf(buffer + 5, "%[^,],%d,%d", ip, &p1, &p2);
        // Rebuilding the Data Port, This would be the port on which the server will try to connect to the client for data transfer.
        int client_data_port = p1 * 256 + p2;

        printf("Client data IP: %s, Port: %d\n", ip, client_data_port);

        // Connect to the client data port
        ftp_data_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (ftp_data_socket < 0)
        {
            perror("Data socket creation failed");
            close(ftp_control_socket);
            return NULL;
        }

        memset(&client_data_addr, 0, sizeof(client_data_addr));
        client_data_addr.sin_family = AF_INET;
        client_data_addr.sin_addr.s_addr = inet_addr(ip);
        client_data_addr.sin_port = htons(client_data_port);

        if (connect(ftp_data_socket, (struct sockaddr *)&client_data_addr, sizeof(client_data_addr)) < 0)
        {
            perror("Data connection failed");
            close(ftp_data_socket);
            close(ftp_control_socket);
            return NULL;
        }

        printf("Data connection established with client.\n");

        // Step 4: Receive LIST Command
        memset(buffer, 0, BUFFER_SIZE);
        read(ftp_control_socket, buffer, BUFFER_SIZE);
        printf("Received Command: %s", buffer);

        if (strncmp(buffer, "LIST", 4) == 0)
        {
            // List files from the user's home directory (e.g., /home/ftpuser)
            list_directory(CLIENT_HOME_DIR, ftp_data_socket);
            close(ftp_data_socket);

            const char *response = "226 Directory send OK.\n";
            write(ftp_control_socket, response, strlen(response));
        }
        else
        {
            const char *error_response = "502 Command not implemented.\n";
            write(ftp_control_socket, error_response, strlen(error_response));
        }
    }

    close(ftp_control_socket);
    printf("Client disconnected.\n");
    return NULL;
}

void list_directory(const char *dir_path, int data_socket)
{
    DIR *dir;
    struct dirent *entry;
    char buffer[BUFFER_SIZE];

    dir = opendir(dir_path);
    if (dir == NULL)
    {
        perror("opendir failed");
        return;
    }

    // Read all entries in the directory and send them to the client
    while ((entry = readdir(dir)) != NULL)
    {
        // Skip hidden files (files starting with .)
        if (entry->d_name[0] != '.')
        {
            snprintf(buffer, sizeof(buffer), "%s\n", entry->d_name);
            write(data_socket, buffer, strlen(buffer));
        }
    }

    closedir(dir);
}

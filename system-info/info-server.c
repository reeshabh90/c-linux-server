#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>

#define MAX_EVENTS 10
#define PORT 8080
#define MESSAGE_INTERVAL 5 // Seconds
#define BUFFER_SIZE 1024
#define PROC_PATH "/proc"
#define MAX_PROCESSES 1024

// Client structure to maintain connection info and outgoing data
typedef struct
{
    int socket_fd;
    char outgoing_data[BUFFER_SIZE];
    int data_ready;
} ClientInfo;

ClientInfo clients[MAX_EVENTS];

// Initialize the client array
void init_clients()
{
    for (int i = 0; i < MAX_EVENTS; i++)
    {
        clients[i].socket_fd = -1;
        clients[i].data_ready = 0;
    }
}

// Add a client to the clients array
int add_client(int new_socket)
{
    for (int i = 0; i < MAX_EVENTS; i++)
    {
        if (clients[i].socket_fd == -1)
        {
            clients[i].socket_fd = new_socket;
            clients[i].data_ready = 0;
            return i;
        }
    }
    return -1; // No space available for new client
}

// Helper function to make socket non-blocking
int make_socket_non_blocking(int sockfd)
{
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags == -1)
    {
        perror("fcntl F_GETFL");
        return -1;
    }

    flags |= O_NONBLOCK;
    if (fcntl(sockfd, F_SETFL, flags) == -1)
    {
        perror("fcntl F_SETFL");
        return -1;
    }
    return 0;
}

// Function to handle sending a message to clients every 5 seconds
void send_message_to_clients(int *clients, int client_count)
{
    const char *message = "Hello from server!\n";
    for (int i = 0; i < client_count; ++i)
    {
        if (clients[i] != -1)
        {
            send(clients[i], message, strlen(message), 0);
        }
    }
}

// Function to log top 5 CPU consuming processes
void get_top_cpu_processes(char *log_data, size_t buffer_size) {
    FILE *fp;
    char process_info[256];

    // Run the ps command to get the top 5 CPU consuming processes
    fp = popen("ps -eo pid,comm,%cpu --sort=-%cpu | head -n 6", "r");
    if (fp == NULL) {
        perror("popen");
        return;
    }

    // Append header
    strncat(log_data, "\nTop 5 CPU Consuming Processes:\n", buffer_size - strlen(log_data) - 1);

    // Skip the header line from ps output
    fgets(process_info, sizeof(process_info), fp);

    // Capture each process line and append to log_data
    while (fgets(process_info, sizeof(process_info), fp) != NULL) {
        strncat(log_data, process_info, buffer_size - strlen(log_data) - 1);
    }

    pclose(fp);
}

// Function to log system information and prepare data to be sent to clients
char log_system_info(struct utsname *sys_info, struct sysinfo *info)
{
    char log_data[BUFFER_SIZE];
    // Clear the log_data buffer
    memset(log_data, 0, BUFFER_SIZE);
    // Prepare system information string
    snprintf(log_data, BUFFER_SIZE, "System Name: %s\n", sys_info->sysname);
    strncat(log_data, "Node Name: ", BUFFER_SIZE - strlen(log_data) - 1);
    strncat(log_data, sys_info->nodename, BUFFER_SIZE - strlen(log_data) - 1);
    strncat(log_data, "\nRelease: ", BUFFER_SIZE - strlen(log_data) - 1);
    strncat(log_data, sys_info->release, BUFFER_SIZE - strlen(log_data) - 1);
    strncat(log_data, "\n", BUFFER_SIZE - strlen(log_data) - 1);

    char uptime_str[64];
    snprintf(uptime_str, sizeof(uptime_str), "Uptime: %ld seconds\n", info->uptime);
    strncat(log_data, uptime_str, BUFFER_SIZE - strlen(log_data) - 1);

    char totalram_str[64];
    snprintf(totalram_str, sizeof(totalram_str), "Total RAM: %lu MB\n", info->totalram / (1024 * 1024));
    strncat(log_data, totalram_str, BUFFER_SIZE - strlen(log_data) - 1);

    char freeram_str[64];
    snprintf(freeram_str, sizeof(freeram_str), "Free RAM: %lu MB\n", info->freeram / (1024 * 1024));
    strncat(log_data, freeram_str, BUFFER_SIZE - strlen(log_data) - 1);

    get_top_cpu_processes(log_data, BUFFER_SIZE);

    // Set data ready for all clients
    for (int i = 0; i < MAX_EVENTS; i++)
    {
        if (clients[i].socket_fd != -1)
        {
            strncpy(clients[i].outgoing_data, log_data, BUFFER_SIZE);
            clients[i].data_ready = 1;
        }
    }
}

void send_data_to_client(ClientInfo *client)
{    
    printf("check data ready\n");
    if (client->data_ready)
    {
        int sent_bytes = send(client->socket_fd, client->outgoing_data, strlen(client->outgoing_data), 0);
        if (sent_bytes > 0)
        {
            client->data_ready = 0; // Reset once the data is sent
        }
        else if (sent_bytes == -1 && errno != EAGAIN)
        {
            perror("Failed to send data to client");
        }
    }
}

// Initialize the server by creating a socket, binding, and listening
void init_server(int *server_fd, struct sockaddr_in *server_addr)
{
    *server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (*server_fd == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(server_addr, 0, sizeof(*server_addr));
    server_addr->sin_family = AF_INET;
    server_addr->sin_addr.s_addr = INADDR_ANY;
    server_addr->sin_port = htons(PORT);

    if (bind(*server_fd, (struct sockaddr *)server_addr, sizeof(*server_addr)) == -1)
    {
        perror("bind");
        close(*server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(*server_fd, MAX_EVENTS) == -1)
    {
        perror("listen");
        close(*server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server initialized and listening on port %d\n", PORT);
}

void create_epoll(int *epoll_fd, int server_fd, struct epoll_event *ev)
{
    // Dereference epoll_fd to store the result
    *epoll_fd = epoll_create1(0);
    if (*epoll_fd == -1)
    {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }

    ev->events = EPOLLIN;    // read events (new connections)
    ev->data.fd = server_fd; // Assign the server file descriptor to the event data
    if (epoll_ctl(*epoll_fd, EPOLL_CTL_ADD, server_fd, ev) == -1)
    {
        perror("epoll_ctl: server_fd");
        exit(EXIT_FAILURE);
    }
}

void handle_new_connection(int epoll_fd, int server_fd, struct epoll_event *ev)
{
    while (1)
    {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);

        if (client_fd == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                // No more connections to accept
                break;
            }
            else
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }
        }

        // Make the client socket non-blocking
        if (make_socket_non_blocking(client_fd) == -1)
        {
            perror("make_socket_non_blocking: client_fd");
            close(client_fd);
            continue;
        }

        // Add the new client socket to epoll
        ev->events = EPOLLIN | EPOLLET; // Edge-triggered mode
        ev->data.fd = client_fd;

        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, ev) == -1)
        {
            perror("epoll_ctl: client_fd");
            close(client_fd);
            continue;
        }

        int client_idx = add_client(client_fd);
        printf("Accepted new connection: FD %d\n", client_fd);
    }
}

int main()
{
    int listen_fd, epoll_fd, event_count;
    struct epoll_event ev, events[MAX_EVENTS];
    struct sockaddr_in server_addr;
    // Initialize all client info
    init_clients();
    struct utsname sys_info;
    struct sysinfo info;

    // Step 1: Create listening socket
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Step 2: Set the listening socket to non-blocking mode
    if (make_socket_non_blocking(listen_fd) == -1)
    {
        perror("make_socket_non_blocking");
        exit(EXIT_FAILURE);
    }

    // Step 3: Bind the socket to the port
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    if (bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    // Step 4: Start listening for connections
    if (listen(listen_fd, SOMAXCONN) == -1)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // Step 5: Create epoll instance
    create_epoll(&epoll_fd, listen_fd, &ev);

    time_t last_message_time = time(NULL);

    // Event loop
    while (1)
    {
        // Get system name information
        if (uname(&sys_info) == -1)
        {
            perror("uname");
            break;
        }
        // Get system uptime
        if (sysinfo(&info) == -1)
        {
            perror("sysinfo");
            break;
            ;
        }
        log_system_info(&sys_info, &info);
        // Step 7: Wait for events using epoll_wait
        event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, 1000); // Timeout of 1000 ms

        if (event_count == -1)
        {
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }
        printf("Event Count : %d\n", event_count);
        for (int i = 0; i < event_count; i++)
        {
            if (events[i].data.fd == listen_fd)
            {
                // Handle new incoming connection
                handle_new_connection(epoll_fd, listen_fd, &ev);
            }
        }
        // Step 8: Send periodic messages to clients every 5 seconds
        time_t current_time = time(NULL);
        if (difftime(current_time, last_message_time) >= MESSAGE_INTERVAL)
        {
            printf("current time: %ld\n", current_time);
            printf("last message time: %ld\n", last_message_time);
            // send_message_to_clients(clients, MAX_EVENTS);
            for (int j = 0; j < MAX_EVENTS; j++)
            {
                if (clients[j].socket_fd != -1)
                {
                    send_data_to_client(&clients[j]);
                }
            }
            last_message_time = current_time;
        }
    }
    // Close the listening socket and epoll instance (though this is never reached)
    close(listen_fd);
    close(epoll_fd);
    return 0;
}

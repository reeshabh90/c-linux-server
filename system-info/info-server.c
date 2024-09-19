/**
 * 📔info-server V1.0📔
 * @author: Reeshabh Choudhary
 * 
 * ℹ️ This program creates an asynhronous server using 'epoll()' system call
 *    and sends system information to connected clients every 5 seconds.
 * 
 * 1. The program first creates a master socket connection via socket() sys call.
 *      By default, socket() is a blocking sys call, and we explicitly make it non-blocking,
 *      so that accept(), recv(), and other socket operations won’t block the program if no data is available.
 *      Instead, they return immediately with an error (e.g., EAGAIN or EWOULDBLOCK).
 * 2. To listen incoming requests, the program uses bind() to run on a specific port on this machine.
 * 3. An epoll instance is created using epoll_create1() system call. Itis a kernel object 
 *      that monitors multiple file descriptors (like sockets)
 *      and returns a file descriptor (epoll_fd) representing this epoll instance.
 * 4. The socket created earlier is added to the epoll instance, so that it can notiy 
 *      when the socket is ready to accept a new connection.
 * 5. The program then enters an infinite ♾️ event loop, where it waits for a new connection
 *      and handles the connection upon coming.
 * 6. The connected client is also added to epoll instance, so that epoll can notify
 *      about read events coming from the client.
 * 7. The program keeps track of time using time() and collects and sends
 *      system information to all connected clients every 5 seconds.
 * 
 */
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

#define MAX_CLIENTS 10 /// < Maximum number of clients allowed
#define PORT 8080 ///< port 8080 will be sued to run the server
#define MESSAGE_INTERVAL 5 /// < 5 Seconds is the message interval
#define BUFFER_SIZE 1024 ///< Buffer limit for data to be sent


/**
 * @brief Client structure to maintain connection info and outgoing data
 * @param socket_fd : File Descriptor associated with the client
 * @param outgoing_data : The data being sent to the client
 * @param data_ready : state to track changes to data
 */
typedef struct
{
    int socket_fd;
    char outgoing_data[BUFFER_SIZE];
    int data_ready;
} ClientInfo;

/// @brief Array of clients with ClientInfo structure, allowing a maximum of MAX_CLIENTS = 10 clients.
ClientInfo clients[MAX_CLIENTS];

/**
 * @brief Function to initialize the clients array
 * @details [LOGIC][CLIENT_INITIALIZATION]
 * 1. Iterate over the content of clients array and for each element do the following:
 *     a. set socket_fd = -1, which will ensure the client item has no File Descriptor attached, 
 *          means available for connection.
 *     b. set data_ready = 0, which is a flag to represent, data has not ye been update for the client.
 * @paragraph: When logging system information (every 5 minutes in this case), 
 * the server needs to send the same information to all connected clients.
 * However, not all clients may be ready to receive data at the exact same moment.
 */
void init_clients()
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        // @ref {CLIENT_INITIALIZATION}
        clients[i].socket_fd = -1;
        clients[i].data_ready = 0;
    }
}

/**
 * @brief Function to add a new client
 * @param new_socket_fd: File descriptor associated with the new client
 * @details [LOGIC][CLIENT_ADDITION]
 * 1. Iterate over the content of clients array and check for the first available item with socket_fd = -1
 * 2. assign new_socket_fd to this item's socket_fd and mark data_ready as false.
 * 3. Ensure there is no overflow of clients beyond allowed limit i.e. 'MAX_CLIENTS'
 */
int add_client(int new_socket_fd)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i].socket_fd == -1)
        {
            clients[i].socket_fd = new_socket_fd;
            clients[i].data_ready = 0;
            return i;
        }
    }
    // @ref {LOGIC}{CLIENT_ADDITION}{3}
    return -1;
}

/**
 * @brief Helper function to to set a socket to non-blocking mode. 
 * @param socket_fd: File Descriptor of corresponding socket
 * 
 * @attention This function ensures that the specified socket operates in non-blocking mode, 
 * meaning that operations such as accept(), recv(), send(), and connect() will return immediately,
 * even if they would normally block (e.g., when no data is available to read or write).
 * @details: [LOGIC][NON_BLOCKING_SOCKET]
 * 1. The fcntl() system call is used here with the F_GETFL command 
 *      to retrieve the current file status flags of the socket.
 * 2. If fcntl() returns -1, it indicates an error, so the program prints 
 *      an error message (perror()) and returns -1.
 * 3. The O_NONBLOCK flag is added (bitwise OR) to the existing flags
 *      to ensure that operations on the socket are non-blocking.
 * 4. The fcntl() function is called again, but this time with the F_SETFL command, 
 *      which sets the file descriptor's flags to the new value, which
 *      includes the O_NONBLOCK flag that was added in previous step.
 * 5. Return 0 if the operation succeeds.
 */
int make_socket_non_blocking(int socket_fd)
{
    int flags = fcntl(socket_fd, F_GETFL, 0);
    if (flags == -1)
    {
        perror("fcntl F_GETFL");
        return -1;
    }

    flags |= O_NONBLOCK;
    if (fcntl(socket_fd, F_SETFL, flags) == -1)
    {
        perror("fcntl F_SETFL");
        return -1;
    }
    return 0;
}

/**
 * @brief This function retrieves and logs information about the top 5 CPU-consuming processes.
 * @attention It uses the popen() function to execute a shell command and capture the output on a Linux system.
 * @param log_data: A character buffer where the function will store the process information. 
 *      The buffer is expected to be pre-allocated before calling this function.
 * @param buffer_size: The size of the log_data buffer to prevent buffer overflows.
 * 
 * @details[LOGIC][CPU_PROCESS]
 * 1. popen() runs the shell command <ps -eo pid,comm,%cpu --sort=-%cpu | head -n 6>, which retrieves the following: 
 *      pid: The process ID 
 *      comm: The command name (executable name of the process)
 *      %cpu: The CPU usage percentage.     
 * 2. Retrive the output of the executed shell command and append to the log_data.
 * 3. After processing the output, the file pointer fp is closed using pclose(), 
 *      which also terminates the ps process that was spawned by popen().  
 */
void get_top_cpu_processes(char *log_data, size_t buffer_size) {
    FILE *fp;
    char process_info[256];
    fp = popen("ps -eo pid,comm,%cpu --sort=-%cpu | head -n 6", "r");
    if (fp == NULL) {
        perror("popen");
        return;
    }
    // append header
    strncat(log_data, "\nTop 5 CPU Consuming Processes:\n", buffer_size - strlen(log_data) - 1);

    // Skip the header line from ps output
    fgets(process_info, sizeof(process_info), fp);

    // @ref {LOGIC}{CPU_PROCESS}{2}
    while (fgets(process_info, sizeof(process_info), fp) != NULL) {
        strncat(log_data, process_info, buffer_size - strlen(log_data) - 1);
    }
    // @ref {LOGIC}{CPU_PROCESS}{3}
    pclose(fp);
}

/**
 * @brief Function to log system information and prepare data to be sent to the clients.
 * @param uts_info: A pointer to a struct utsname, which contains system-related information (e.g., system name, release version).
 * @param sys_info: A pointer to a struct sysinfo, which holds system statistics (e.g., uptime, total RAM, free RAM).
 * 
 * @details [LOGIC][LOG_PREPARE_DATA]
 * 1. Use snprintf (to convert the numeric values to strings) and strncat to prepare log_data.
 * 2. Information is extracted via pointers uts_info and sys_info.
 * 3. CPU usgae information is extracted and logged via get_top_cpu_processes().
 * 4. For each client item in clients array, having valid File Descriptor:
 *      i. attach logged data.
 *      ii. mark the item with data_ready state as true.
 */
char log_system_info(struct utsname *uts_info, struct sysinfo *sys_info)
{
    char log_data[BUFFER_SIZE];
    // Clear the log_data buffer
    memset(log_data, 0, BUFFER_SIZE);
    // @ref {LOGIC}{LOG_PREPARE_DATA}{1,2}
    snprintf(log_data, BUFFER_SIZE, "System Name: %s\n", uts_info->sysname);
    strncat(log_data, "Node Name: ", BUFFER_SIZE - strlen(log_data) - 1);
    strncat(log_data, uts_info->nodename, BUFFER_SIZE - strlen(log_data) - 1);
    strncat(log_data, "\nRelease: ", BUFFER_SIZE - strlen(log_data) - 1);
    strncat(log_data, uts_info->release, BUFFER_SIZE - strlen(log_data) - 1);
    strncat(log_data, "\n", BUFFER_SIZE - strlen(log_data) - 1);

    char uptime_str[64];
    snprintf(uptime_str, sizeof(uptime_str), "Uptime: %ld seconds\n", sys_info->uptime);
    strncat(log_data, uptime_str, BUFFER_SIZE - strlen(log_data) - 1);

    char totalram_str[64];
    snprintf(totalram_str, sizeof(totalram_str), "Total RAM: %lu MB\n", sys_info->totalram / (1024 * 1024));
    strncat(log_data, totalram_str, BUFFER_SIZE - strlen(log_data) - 1);

    char freeram_str[64];
    snprintf(freeram_str, sizeof(freeram_str), "Free RAM: %lu MB\n", sys_info->freeram / (1024 * 1024));
    strncat(log_data, freeram_str, BUFFER_SIZE - strlen(log_data) - 1);
    // @ref {LOGIC}{LOG_PREPARE_DATA}{3}
    get_top_cpu_processes(log_data, BUFFER_SIZE);

    // @ref {LOGIC}{LOG_PREPARE_DATA}{4}
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i].socket_fd != -1)
        {
            strncpy(clients[i].outgoing_data, log_data, BUFFER_SIZE);
            clients[i].data_ready = 1;
        }
    }
}

/**
 * @brief Function to send data to cleints
 * @param client: Apointer to an item inside clients array with ClientInfo structure
 * 
 * @details [LOGIC][SEND_DATA]
 * 1. Check whether the client is in a data ready state
 * 2. use send() sys call by providing client's socket File Descriptor and attached data.
 * 3. reset the data_ready flag
 */

void send_data_to_client(ClientInfo *client)
{    
    /// @ref {LOGIC}{SEND_DATA}{1}
    if (client->data_ready)
    {
        // @ref {LOGIC}{SEND_DATA}{2}
        int sent_bytes = send(client->socket_fd, client->outgoing_data, strlen(client->outgoing_data), 0);
        if (sent_bytes > 0)
        {
            // @ref {LOGIC}{SEND_DATA}{3}
            client->data_ready = 0; // Reset once the data is sent
        }
        else if (sent_bytes == -1 && errno != EAGAIN)
        {
            perror("Failed to send data to client");
        }
    }
}

/**
 * @brief This function creates an epoll instance for provided server File Descriptor
 * @param epoll_fd: A pointer to file descriptor corresponding to epoll
 * @param server_fd: Server socket File Descriptor
 * @param ev: A pointer to struct epoll_event 
 */
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

/**  
 * @brief This function handles incoming connection from clients
 * @param epoll_fd File descriptor corresponding to epoll instance
 * @param server_fd File descriptor corresponding to server socket
 * @param ev A pointer to struct epoll_event 
 * @details [LOGIC][HANDLE_CONNECTION] 
 * 1. Use accept() system call to accept client connection
 * 2. Make the socket corresponding to new connection as non-blocking
 * 3. Add the client socket to epoll instance
 * 4. Add the client in the clients array
 */
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

/// @brief The main function of the program
/// @return 0 if everything runs successfully.
int main()
{
    int server_fd, epoll_fd, event_count;
    struct epoll_event ev, events[MAX_CLIENTS];
    struct sockaddr_in server_addr;
    // Initialize all client info
    init_clients();
    struct utsname sys_info;
    struct sysinfo info;

    
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    
    if (make_socket_non_blocking(server_fd) == -1)
    {
        perror("make_socket_non_blocking");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, SOMAXCONN) == -1)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    create_epoll(&epoll_fd, server_fd, &ev);

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
        event_count = epoll_wait(epoll_fd, events, MAX_CLIENTS, 1000); // Timeout of 1000 ms

        if (event_count == -1)
        {
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }
        printf("Event Count : %d\n", event_count);
        for (int i = 0; i < event_count; i++)
        {
            if (events[i].data.fd == server_fd)
            {
                // Handle new incoming connection
                handle_new_connection(epoll_fd, server_fd, &ev);
            }
        }
        time_t current_time = time(NULL);
        if (difftime(current_time, last_message_time) >= MESSAGE_INTERVAL)
        {
            printf("current time: %ld\n", current_time);
            printf("last message time: %ld\n", last_message_time);
            for (int j = 0; j < MAX_CLIENTS; j++)
            {
                if (clients[j].socket_fd != -1)
                {
                    send_data_to_client(&clients[j]);
                }
            }
            last_message_time = current_time;
        }
    }
    close(server_fd);
    close(epoll_fd);
    return 0;
}

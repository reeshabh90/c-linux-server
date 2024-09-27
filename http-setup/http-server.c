/**
 * http-server V1.0📔
 * @file: http-server.c
 * @author: Reeshabh Choudhary
 *
 * ℹ️ This program creates an asynhronous HTTP server using 'epoll()'.
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
 * 7. The program expects messages adhering to HTTP protocol.
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
#include <arpa/inet.h>

#define MAX_CLIENTS 10     ///< Maximum number of clients allowed.
#define MAX_HEADERS 20     ///< Maximum number of headers allowed.
#define PORT 8080          ///< port 8080 will be sued to run the server.
#define MESSAGE_INTERVAL 5 ///< 5 Seconds is the message interval.
#define BUFFER_SIZE 4096   ///< Buffer limit for data to be sent.

/**
 * @brief Header structure for HTTP request headers
 */
typedef struct
{
    char *key;
    char *value;
} Header;

/**
 * @brief HTTP Request structure
 */
typedef struct
{
    char method[10];
    char uri[256];
    char version[10];
    Header headers[MAX_HEADERS];
    int header_count;
    char *body;
} HttpRequest;

/**
 * @brief HTTP response structure
 */
typedef struct
{
    int status_code;
    char *status_text;
    Header headers[MAX_HEADERS];
    int header_count;
    char *body;
} HttpResponse;

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
 * @brief This function creates an epoll instance for provided server File Descriptor
 * @param epoll_fd: A pointer to file descriptor corresponding to epoll
 * @param server_fd: Server socket File Descriptor
 * @param ev: A pointer to struct epoll_event
 * @details [LOGIC][EPOLL_INSTANCE]
 * 1. Create an epoll instance using epoll_create1()
 * 2. The server file descriptor 'server_fd' is added
 *      to epoll instance using 'epoll_ctl()' system call.
 * 3. ev, the 'epoll_event' data structure is used to signify the event types we are interested.
 * 4. once a file descriptor is added 'epoll_wait() will notify our program when new events
 *      (like connection requests) occur on the listening socket.
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
 * 3. Add the client socket to epoll instance' to get notified about events
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

/**
 * @brief This function adds the Response Header Key/Value pair to the HTTP response
 * @param response Pointer to the original response body
 * @param key
 * @param value
 * @details [LOGIC][ADD_RESPONSE_HEADER]
 * 1. Check if header count does not exceed maximum headers
 * 2. Add KEY/VALUE to respective parameters of HttpResponse
 */
void add_response_header(HttpResponse *response, const char *key, const char *value)
{
    if (response->header_count < MAX_HEADERS)
    {
        response->headers[response->header_count].key = strdup(key);
        response->headers[response->header_count].value = strdup(value);
        response->header_count++;
    }
}
/**
 * @brief This function parses the HTTP request text coming from client
 */
void parse_request_line(char *line, HttpRequest *request)
{
    sscanf(line, "%s %s %s", request->method, request->uri, request->version);
}

void parse_header(char *line, Header *header)
{
    char *colon = strchr(line, ':');
    if (colon)
    {
        *colon = '\0';
        header->key = strdup(line);
        header->value = strdup(colon + 2); // +2 to skip ': '
    }
}

/**
 * @brief This function parses a HTTP request and
 * breaks down the raw request data into its component parts: the request line, headers, and body.
 * @param buffer The received text from the client read during 'handle_read_operation'.
 * @param request Pointer to the HttpRequest body/structure.
 * @details [LOGIC][PARSE_REQUEST]
 * 1. Split the buffer at the first occurrence of "\r\n"
 * (carriage return followed by line feed, which separates lines in HTTP). It returns first line of HTTP request.
 * 2. Parse the above parsed line into method, URI, and HTTP version, and store them in the HttpRequest structure.
 * 3. Continue reading subsequent lines and check if line is not empty.
 * 4. For each non emepty lines, we check the following:
 *      i.      MAX_HEADERS count has not exceeded
 *      ii.     Parse and extract header line into 'headers' array of HttpRequest structure.
 *      iii.    Keep incrementing header count to extract all headers
 * 5. After processing all headers, parse the remaining part of the buffer, which is the request body.
 * 6. Store the request body in HttpRequest structure's request parameter.
 *
 */
void parse_request(char *buffer, HttpRequest *request)
{
    // {ref}{LOGIC}{PARSE_REQUEST}{1,2}
    char *line = strtok(buffer, "\r\n");
    parse_request_line(line, request);

    request->header_count = 0;
    // {ref}{LOGIC}{PARSE_REQUEST}{3,4}
    while ((line = strtok(NULL, "\r\n")) && *line)
    {
        if (request->header_count < MAX_HEADERS)
        {
            parse_header(line, &request->headers[request->header_count++]);
        }
    }
    // {ref}{LOGIC}{PARSE_REQUEST}{5,6}
    request->body = strtok(NULL, "");
}

/**
 * @brief Handles the processing of an HTTP GET request by constructing an appropriate response.
 * @param request Pointer to the parsed HttpRequest structure containing the details of the incoming GET request.
 * @param response Pointer to the HttpResponse structure where the response data will be populated.
 * @details [LOGIC][HANDLE_GET_REQUEST]
 * 1. Set the response status code to 200, indicating a successful request.
 * 2. Set the response status text to "OK".
 * 3. Add a "Content-Type" header with a value of "text/html" to the response.
 * 4. Set the body of the response to a simple HTML message.
 */
void handle_get_request(HttpRequest *request, HttpResponse *response)
{
    response->status_code = 200;
    response->status_text = "OK";
    add_response_header(response, "Content-Type", "text/html");
    response->body = "<html><body><h1>Hello from GET!</h1></body></html>";
}

/**
 * @brief Handles the processing of an HTTP POST request by constructing an appropriate response.
 * @param request Pointer to the parsed HttpRequest structure containing the details of the incoming POST request.
 * @param response Pointer to the HttpResponse structure where the response data will be populated.
 * @details [LOGIC][HANDLE_POST_REQUEST]
 * 1. Set the response status code to 200, indicating a successful request.
 * 2. Set the response status text to "OK".
 * 3. Add a "Content-Type" header with a value of "text/plain" to the response.
 * 4. Set the body of the response to acknowledge receipt of the POST request.
 */
void handle_post_request(HttpRequest *request, HttpResponse *response)
{
    response->status_code = 200;
    response->status_text = "OK";
    add_response_header(response, "Content-Type", "text/plain");
    response->body = "Received POST request";
}

/**
 * @brief Sends the constructed HTTP response to the client over the specified socket file descriptor.
 * @param client_fd The file descriptor for the client's socket connection.
 * @param response Pointer to the HttpResponse structure containing the status, headers, and body to be sent.
 * @details [LOGIC][SEND_RESPONSE]
 * 1. Use `snprintf` to format the response status line with the HTTP version, status code, and status text.
 * 2. Loop through the headers in the response structure and append each header (key-value pairs) to the response buffer.
 * 3. Append a blank line (`\r\n`) to separate headers from the body.
 * 4. If the response contains a body, append it to the buffer.
 * 5. Send the complete response buffer to the client using the `send` system call.
 */
void send_response(int client_fd, HttpResponse *response)
{
    char buffer[BUFFER_SIZE];
    int len = snprintf(buffer, BUFFER_SIZE, "HTTP/1.1 %d %s\r\n",
                       response->status_code, response->status_text);

    for (int i = 0; i < response->header_count; i++)
    {
        len += snprintf(buffer + len, BUFFER_SIZE - len, "%s: %s\r\n",
                        response->headers[i].key, response->headers[i].value);
    }

    len += snprintf(buffer + len, BUFFER_SIZE - len, "\r\n");

    if (response->body)
    {
        len += snprintf(buffer + len, BUFFER_SIZE - len, "%s", response->body);
    }

    send(client_fd, buffer, len, 0);
}

/**
 * @brief Handles reading data from a client socket and processing the HTTP request.
 * @param epoll_fd The file descriptor for the epoll instance to manage client connections.
 * @param client_fd The file descriptor for the client socket connection.
 * @details [LOGIC][HANDLE_READ_OPERATION]
 * 1. Read incoming data from the client into a buffer using the `read` system call.
 * 2. If no bytes are read, assume the client has disconnected and remove the client from the epoll instance.
 * 3. If data is read, print the received request and ensure the buffer is null-terminated.
 * 4. Parse the HTTP request from the received data using `parse_request`.
 * 5. Determine the HTTP method and call the appropriate handler (`handle_get_request` for GET, `handle_post_request` for POST).
 * 6. If the method is unsupported, set the response to status 405 (Method Not Allowed).
 * 7. Send the constructed HTTP response back to the client using `send_response`.
 */
void handle_read_operation(int epoll_fd, int client_fd)
{
    char buffer[BUFFER_SIZE] = {0};
    int bytes_read = read(client_fd, buffer, BUFFER_SIZE);
    if (bytes_read == 0)
    {
        // Client disconnected
        printf("Client disconnected\n");
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
    }
    else if (bytes_read > 0)
    {
        printf("Received from client: %s\n", buffer);
        buffer[bytes_read] = '\0';

        HttpRequest request;
        HttpResponse response = {0};

        parse_request(buffer, &request);

        if (strcmp(request.method, "GET") == 0)
        {
            handle_get_request(&request, &response);
        }
        else if (strcmp(request.method, "POST") == 0)
        {
            handle_post_request(&request, &response);
        }
        else
        {
            response.status_code = 405;
            response.status_text = "Method Not Allowed";
            add_response_header(&response, "Content-Type", "text/plain");
            response.body = "Unsupported method";
        }
        send_response(client_fd, &response);
    }
}

/**
 * @brief The entry point of the server application that sets up socket communication, initializes epoll, and handles client connections.
 * @details [LOGIC][MAIN]
 * 1. Create a non-blocking server socket for handling incoming client connections.
 * 2. Configure the socket address (IPv4, any incoming address, and specified port) and bind it to the server socket.
 * 3. Start listening for incoming connections with a backlog defined by `SOMAXCONN`.
 * 4. Create and initialize an epoll instance to monitor events on the server socket and client connections.
 * 5. Enter an event loop that waits for events using `epoll_wait` with a 1000ms timeout.
 *    - If the event corresponds to the server socket, handle new incoming connections.
 *    - If the event corresponds to an existing client connection and it is ready for reading (`EPOLLIN`), handle the read operation.
 * 6. On server shutdown, close both the server and epoll file descriptors.
 * @return Returns 0 on normal termination, or exits with failure status if errors occur.
 */
int main()
{
    int server_fd, epoll_fd, event_count;
    struct epoll_event ev, events[MAX_CLIENTS];
    struct sockaddr_in server_addr;
    // {ref}{LOGIC}{MAIN}{1}
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

    // {ref}{LOGIC}{MAIN}{2}
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    // {ref}{LOGIC}{MAIN}{3}
    if (listen(server_fd, SOMAXCONN) == -1)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // {ref}{LOGIC}{MAIN}{4}
    create_epoll(&epoll_fd, server_fd, &ev);

    // {ref}{LOGIC}{MAIN}{5}
    while (1)
    {
        // Wait for events on monitored file descriptors with a timeout of 1000ms
        event_count = epoll_wait(epoll_fd, events, MAX_CLIENTS, 1000);

        if (event_count == -1)
        {
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < event_count; i++)
        {
            if (events[i].data.fd == server_fd)
            {
                // Handle new incoming client connection
                handle_new_connection(epoll_fd, server_fd, &ev);
            }
            else
            {
                if (events[i].events & EPOLLIN)
                {
                    // Handle readable client socket (incoming data)
                    handle_read_operation(epoll_fd, events[i].data.fd);
                }
            }
        }
    }

    // {ref}{LOGIC}{MAIN}{6}
    close(server_fd);
    close(epoll_fd);
    return 0;
}

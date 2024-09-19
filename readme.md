# Client Server program in C
## OS: Unix or Linux based system

This is a sample project where I am trying to learn how to create Server and Client in Inter Process Communication on same machine 
and Network Communication on two physical machines.

## Project Details
This repository has multiple projects inside it. 
1. basic-server : This project has a sample code for client-server connection using Unix-domain-socket.
2. multiplex-server : This project has a sample code for server which can connect to multiple client machines and perform computation for them.
3. multiplex-routinginfo : This project has sample code for 1 server and 2 types of client: routing_update_client & routing_client.
    * routing_update_client can add routing entries to routing server and see the current list maintained by server.
    * routing_client can just see the list of routing entries in the server. List gets updated whenever new entry is added.
4. system-info: This project has sample code for an asynchronous server which makes use of 'epoll' to asynchronously connect with clients and shares system information every 5 second. We get a basic idea about how event loop functions.

## How to run the project
Suppose you are in sample project multiplex-routinginfo. Please run following commands:
<gcc -c routing_server.c -o routing_server.o>
<gcc -c routing_table.c -o routing_table.o>
<gcc routing_server.c routing_table.o -o routing_server>
<gcc -g routing_client.c -o routing_client>
<gcc -g routing_update_client.c -o routing_update_client>

## Concept
Server maintains client connection using File Descriptors. File descriptors are integers that uniquely identify open files within a process. When a file is opened, the operating system assigns a non-negative integer to represent that file within the context of the process.
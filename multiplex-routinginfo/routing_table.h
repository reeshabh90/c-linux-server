#ifndef ROUTING_TABLE_H
#define ROUTING_TABLE_H

#define MAX_ROUTE_TABLE_ENTRY 10

// Define a new type name 'RouteEntry' for the struct
typedef struct {
    char destination[16];  // IPv4 address in dotted decimal notation
    char mask[16];         // Subnet mask
    char gateway[16];      // Gateway IP
    char oif[10];          // Outgoing interface name
} RouteEntry;

// Routing table functions
void init_routing_table();
int add_route(RouteEntry new_entry);
int update_route(char *destination, RouteEntry updated_entry);
int delete_route(char *destination);
void print_routing_table();
int get_routing_table_size();
RouteEntry* get_routing_table();
int check_table_change();
void reset_table_changed();

#endif  // ROUTING_TABLE_H
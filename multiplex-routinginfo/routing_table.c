#include "routing_table.h"
#include <stdio.h>
#include <string.h>

// Global routing table and size
RouteEntry routing_table[MAX_ROUTE_TABLE_ENTRY];
int table_size = 0;
int table_change_flag = 0;

/// @brief Initialize the routing table
void init_routing_table() {
    table_size = 0;
    table_change_flag = 0;
}

/// @brief Add a new route to the table
int add_route(RouteEntry new_entry) {
    if (table_size < MAX_ROUTE_TABLE_ENTRY) {
        routing_table[table_size] = new_entry;
        table_size++;
        return 1;
    }
    return 0;
}

/// @brief Update an existing route
int update_route(char *destination, RouteEntry updated_entry) {
    for (int i = 0; i < table_size; i++) {
        if (strcmp(routing_table[i].destination, destination) == 0) {
            routing_table[i] = updated_entry;
            return 1;
        }
    }
    return 0;
}

/// @brief Delete a route from the table
int delete_route(char *destination) {
    for (int i = 0; i < table_size; i++) {
        if (strcmp(routing_table[i].destination, destination) == 0) {
            routing_table[i] = routing_table[table_size - 1];  // Replace with last entry
            table_size--;
            return 1;
        }
    }
    return 0;
}

/// @brief Print the routing table
void print_routing_table() {
    for (int i = 0; i < table_size; i++) {
        printf("Destination: %s, Mask: %s, Gateway: %s, OIF: %s\n", 
               routing_table[i].destination, routing_table[i].mask, 
               routing_table[i].gateway, routing_table[i].oif);
    }
}

/// @brief Get the current size of the routing table
int get_routing_table_size() {
    return table_size;
}

/// @brief Get the routing table entries (for server use)
RouteEntry* get_routing_table() {
    return routing_table;
}


/// @brief A function to check if the routing table has changed
int check_table_change() {
    return table_change_flag;
}

/// @brief A function to reset the change flag
void reset_table_changed() {
    table_change_flag = 0;
}
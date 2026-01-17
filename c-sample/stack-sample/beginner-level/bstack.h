#ifndef STACK_H
#define STACK_H

#define STACK_MAX_SIZE 100
#define NOT_AVAILABLE -1

typedef struct
{
    int top;
    int items[STACK_MAX_SIZE];
} stack;

#endif

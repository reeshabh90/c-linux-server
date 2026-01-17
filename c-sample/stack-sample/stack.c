#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "stack.h"

#define NOT_AVAILABLE -1
#define STACK_MAX_CAPACITY 100
#define STACK_INVALID_VALUE -999999999

struct Stack
{
    int top;
    int items[STACK_MAX_CAPACITY];
};

Stack *stack_create()
{
    Stack *stk = malloc(sizeof(Stack));
    if (!stk)
    {
        return NULL;
    }
    stk->top = NOT_AVAILABLE;
    return stk;
}

void stack_destroy(Stack **stk)
{
    if (!stk)
    {
        return;
    }
    free(*stk);
    *stk = NULL;
}

bool stack_is_empty(const Stack *stk)
{
    return (stk && stk->top < 0);
}

bool stack_is_full(const Stack *stk)
{
    return (stk && stk->top >= STACK_MAX_CAPACITY - 1);
}

size_t stack_size(const Stack *stk)
{
    return stk ? (size_t)(stk->top + 1) : 0;
}

void stack_print(const Stack *stk)
{
    if (!stk)
    {
        printf("Null Invalid Stack\n");
        return;
    }

    if (stack_is_empty(stk))
    {
        printf("Stack is empty\n");
        return;
    }

    printf("Stack (size: %zu, top → bottom):\n", stack_size(stk));
    for (int i = stk->top; i >= 0; i--)
    {
        printf("  [%2d] = %d\n", i, stk->items[i]);
    }
}

bool stack_push(Stack *stk, int value)
{
    if (!stk || stack_is_full(stk))
    {
        return false;
    }

    stk->items[++(stk->top)] = value;
    return true;
}

int stack_pop(Stack *stk)
{
    if (!stk || stack_is_empty(stk))
    {
        return false;
    }
    return stk->items[stk->top--];
}

int stack_peek(const Stack *stk)
{
    if (!stk || stack_is_empty(stk))
    {
        return false;
    }
    return stk->items[stk->top];
}

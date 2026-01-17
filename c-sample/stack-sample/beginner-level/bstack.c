#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "bstack.h"

void print_stack(const stack *stk)
{
    if (stk->top == NOT_AVAILABLE)
    {
        printf("Stack is empty\n");
        return;
    }

    printf("Stack (top to bottom):\n");
    for (int i = stk->top; i >= 0; i--)
    {
        printf("  %d\n", stk->items[i]);
    }
    printf("Size: %d\n\n", stk->top + 1);
}

void push(int item, stack *stk)
{
    if (stk->top >= STACK_MAX_SIZE - 1)
    {
        printf("Stack Overflow!\n");
        return;
    }

    stk->items[++(stk->top)] = item;
    printf("Pushed: %d  (new size: %d)\n", item, stk->top + 1);
}

void pop(stack *stk)
{
    if (stk->top == NOT_AVAILABLE)
    {
        printf("Stack Underflow!\n");
        return;
    }

    int value = stk->items[(stk->top)--];
    printf("Popped: %d\n", value);
}

int main()
{
    stack stk = {
        .top = NOT_AVAILABLE,
        .items = {0}};

    srand(time(NULL));
    int min = 1, max = 100;

    printf("Pushing 10 random numbers:\n");
    for (int i = 0; i < 10; i++)
    {
        int num = (rand() % (max - min + 1)) + min;
        push(num, &stk);
    }

    printf("\nFinal stack:\n");
    print_stack(&stk);

    printf("\nPopping one element:\n");
    pop(&stk);

    printf("\nStack after pop:\n");
    print_stack(&stk);

    return EXIT_SUCCESS;
}

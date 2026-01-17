#include "stack.h"
#include <stdio.h>
#include <stdlib.h>

int main()
{
    Stack *stk = stack_create();
    if (!stk)
    {
        fprintf(stderr, "Stack creation failed\n");
        return EXIT_FAILURE;
    }

    for (int i = 10; i <= 100; i += 10)
    {
        if (!stack_push(stk, i))
        {
            printf("Push failed at %d\n", i);
            break;
        }
    }
    stack_print(stk);
    int popped_val = stack_pop(stk);
    printf("Popped Value from Stack: %d\n", popped_val);
    printf("Top most value of the stack currently is: %d\n", stack_peek(stk));

    popped_val = stack_pop(stk);
    printf("Popped Value from Stack: %d\n", popped_val);
    printf("Top most value of the stack currently is: %d\n", stack_peek(stk));
}

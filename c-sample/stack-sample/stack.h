#ifndef STACK_H
#define STACK_H

#include <stddef.h>
#include <stdbool.h>
// forward declaration
typedef struct Stack Stack;

/**
 * @brief Create a new stack instance on Heap, 🔴 Remember to Destroy~!
 * @return pointer to new stack or NULL on failure
 */
Stack *stack_create(void);

/**
 * @brief Destroy stack and free all resources
 * @param s double pointer — will be set to NULL after destruction
 */
void stack_destroy(Stack **stk);

/**
 * @brief Push value onto stack
 * @return true on success, false if stack is full or invalid
 */
bool stack_push(Stack *stk, int value);

/**
 * @brief Pop value from stack
 * @param stk referenced Stack
 * @return int if pop succeeded, return popped value
 */
int stack_pop(Stack *stk);

/**
 * @brief Get value from top of stack without removing it
 * @param stk referenced Stack
 * @return int if peek succeeded, return top value
 */
int stack_peek(const Stack *stk);

/**
 * @brief Check if stack is empty
 */
bool stack_is_empty(const Stack *stk);

/**
 * @brief Check if stack is full
 */
bool stack_is_full(const Stack *stk);

/**
 * @brief Get current number of elements
 */
size_t stack_size(const Stack *stk);

/**
 * @brief Print stack contents
 */
void stack_print(const Stack *stk);

#endif

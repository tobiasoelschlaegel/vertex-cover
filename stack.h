#ifndef STACK_H_INCLUDED
#define STACK_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

/**
 * @defgroup Stack Stack
 * @{
 */

struct _stack_s
{
    /** @brief Number of elements that are on the stack. */
    uint32_t n;
    /** @brief Size in bytes of one element. */
    uint32_t size;
    /** @brief Number of slots that are allocated. */
    uint32_t allocated;
    /** @brief Pointer to raw data of stack. */
    uint8_t *data;
};
typedef struct _stack_s stack_t;

void stack_init(stack_t *stack, uint32_t size);
void stack_destroy(stack_t *stack);
void stack_push(stack_t *stack, void *data);
bool stack_isempty(const stack_t const *stack);
uint32_t stack_height(const stack_t const *stack);
bool stack_pop(stack_t *stack, void *data);
bool stack_top(stack_t *stack, void *data);
void stack_sort(stack_t *stack, int (*compar)(const void *, const void *));
bool stack_binsearch(stack_t *stack, int (*compar)(const void *, const void *), const void *data, uint32_t *position);
bool stack_find_max(stack_t *stack, int (*compar)(const void *, const void *), void *result);
bool stack_get_element(stack_t *stack, uint32_t position, void *result);
void stack_remove_element(stack_t *stack, int (*compar)(const void *, const void *), void *data);
void stack_remove_last(stack_t *stack);
bool stack_contains(stack_t *stack, int (*compar)(const void *, const void *), void *data);
void *stack_get_element_ptr(const stack_t const *stack, uint32_t position);

/** @} */

#endif


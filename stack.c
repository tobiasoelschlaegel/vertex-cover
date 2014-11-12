#include <assert.h>
#include "stack.h"

/**
 * @brief Initializes a stack.
 * @param stack Pointer to uninitialized stack.
 * @param size Size in bytes of elements the stack needs to contain.
 */
void stack_init(stack_t *stack, uint32_t size)
{
    assert(stack);
    assert(size > 0);
    
    stack->n = 0;
    stack->size = size;
    stack->allocated = 10;
    stack->data = (uint8_t *) malloc(sizeof(uint8_t) * stack->allocated * stack->size);
    assert(stack->data);
}

/**
 * @brief Destroys a stack.
 * @details Releases all memory allocated for storing elements.
 * @param stack Stack to destroy.
 */
void stack_destroy(stack_t *stack)
{
    assert(stack);
    if(stack->data)
        free(stack->data);
    stack->data = NULL;
}

/**
 * @brief Pushes an element onto the stack.
 * @details If a stack with element size `sizeof(uint32_t)` was initialized and the value `uint32_t x = 10` is to be pushed onto the stack, `stack_push` needs to be called with `&x`.
 * @param stack Stack to push the element onto.
 * @param data Pointer to element to push.
 */
void stack_push(stack_t *stack, void *data)
{
    assert(stack);
    assert(data);
    
    if(stack->n == stack->allocated)
    {
        uint8_t *new_data = (uint8_t *) realloc(stack->data, 2 * sizeof(uint8_t) * stack->size * stack->allocated);
        if(new_data == NULL)
            return;

        stack->data = new_data;
        stack->allocated = stack->allocated * 2;
    }
    
    memcpy(stack->data + stack->n * stack->size, data, stack->size);
    stack->n = stack->n + 1;
}

/**
 * @brief Tests whether a stack is empty.
 * @param stack Stack to be tested.
 * @returns `true` if the stack is empty, `false` if it contains elements.
 */
bool stack_isempty(const stack_t const *stack)
{
    assert(stack);
    return (stack->n == 0);
}

/**
 * @brief Returns the number of elements that are currently on a stack.
 * @param stack Stack
 * @returns Number of elements that are on the stack.
 */
uint32_t stack_height(const stack_t const *stack)
{
    //assert(stack);
    return stack->n;
}

/**
 * @brief Pops an element from a stack.
 * @param stack Stack to pop the element from.
 * @param data Address to store the data of the element that is on top.
 * @returns `true` if an element was removed, `false` if the stack was empty.
 */
bool stack_pop(stack_t *stack, void *data)
{
    assert(stack);
    assert(data);
    
    if(stack->n == 0)
        return false;
    
    memcpy(data, stack->data + (stack->n - 1) * stack->size, stack->size);
    stack->n = stack->n - 1;
    
    /* TODO: release space if load too small */
    return true;
}

/**
 * @brief Pop without removing the element.
 * @param stack Stack.
 * @param data Address to store the data.
 * @returns `true` if data could be copied, `false` if the stack was empty.
 */
bool stack_top(stack_t *stack, void *data)
{
    assert(stack);
    assert(data);
    
    if(stack->n == 0)
        return false;
    
    memcpy(data, stack->data + (stack->n - 1) * stack->size, stack->size);
    return true;
}

/**
 * @brief Sorts the elements of a stack.
 * @details See the manpage of `qsort` for details on the comparison function.
 * @param stack Stack to sort.
 * @param compar Function that compares two elements.
 */
void stack_sort(stack_t *stack, int (*compar)(const void *, const void *))
{
    assert(stack);
    assert(compar);
    
    qsort(stack->data, stack->n, stack->size, compar);
}

/**
 * @brief Finds the position of an element in a sorted stack.
 * @param stack Sorted stack to search in.
 * @param compar Functino that compares two elements.
 * @param data Element that is to be found.
 * @param position Address to store the position of the element in.
 * @returns `true` if the element was found, `false` if not.
 */
bool stack_binsearch(stack_t *stack, int (*compar)(const void *, const void *), const void *data, uint32_t *position)
{
    uint32_t l, r, m;
    int res;
    
    assert(stack);
    assert(compar);
    
    l = 0;
    r = stack_height(stack);
    
    while(l < r)
    {
        m = (l + r) / 2;
        res = compar(stack->data + m * stack->size, data);
        
        if(res < 0)
            l = m + 1;
        else if(res > 0)
            r = m;
        else
        {
            if(position)
                *position = m;
            return true;
        }
    }

    return false;
}

/**
 * @brief Finds the maximum element of a stack.
 * @details Scans through all elements and uses `compar` to find a maximum element.
 * @param stack Stack.
 * @param compar Function that compares two elements.
 * @param result Address to copy data of maximum element to.
 * @returns `true` if a maximum element could be found, `false` if the stack was empty.
 */
bool stack_find_max(stack_t *stack, int (*compar)(const void *, const void *), void *result)
{
    uint32_t i, height;
    
    assert(stack);
    assert(compar);
    assert(result);
    
    height = stack_height(stack);
    if(height == 0)
        return false;
    
    memcpy(result, stack->data, stack->size);
    for(i = 1; i < height; i++)
    {
        if(compar(result, stack->data + i * stack->size) < 0)
            memcpy(result, stack->data + i * stack->size, stack->size);
    }

    return true;
}

/**
 * @brief Copies data of an element at a specified position of the stack.
 * @details The first element that was pushed onto the stack has position `0`, the last has position `height - 1`.
 * @param stack Stack.
 * @param position Position of the element, needs to be smaller than the height of the stack.
 * @param result Address to copy the data of the element to.
 * @returns `true` if the data could be copied, `false` otherwise.
 */
bool stack_get_element(stack_t *stack, uint32_t position, void *result)
{
    uint32_t height;
    
    assert(stack);
    assert(result);
    
    height = stack_height(stack);
    if(height == 0)
        return false;
    else if(position >= height)
        return false;
    
    memcpy(result, stack->data + position * stack->size, stack->size);
    return true;
}

/**
 * @brief Removes an element from the stack at a specified position.
 * @details Replaces the element with the data of the element that was last pushed onto the stack.
 * @param stack Stack.
 * @param compar Function that compares two elements.
 * @param data Element that is to be removed from the stack.
 */
void stack_remove_element(stack_t *stack, int (*compar)(const void *, const void *), void *data)
{
    uint32_t i;
    void *tmp_data;
    assert(stack);
    assert(compar);
    assert(data);
    
    tmp_data = malloc(stack->size);
    if(tmp_data == NULL)
    {
        fprintf(stderr, "Error: could not allocate memory to store temp stack data\n");
        exit(0);
    }
    
    i = 0;
    while(i < stack_height(stack))
    {
        stack_get_element(stack, i, tmp_data);
        if(compar(tmp_data, data) != 0)
            i++;
        else
        {
            /* put last element into this spot */
            stack_pop(stack, tmp_data);
            if(i < stack_height(stack))
                memcpy(stack->data + i * stack->size, tmp_data, stack->size);
        }
    }
    
    free(tmp_data);
}

/**
 * @brief Removes the last element from the stack.
 * @param stack Stack.
 */
void stack_remove_last(stack_t *stack)
{
    assert(stack);
    
    if(stack_height(stack) > 0)
        stack->n = stack->n - 1;
}

/**
 * @brief Tests whether a stack contains a certain element.
 * @param stack Stack.
 * @param compar Function that compares two elements.
 * @param data Element to be checked for containment.
 * @returns `true` if the element is contained in the stack, `false` otherwise.
 */
bool stack_contains(stack_t *stack, int (*compar)(const void *, const void *), void *data)
{
    uint32_t i, height;
    
    assert(stack);
    assert(compar);
    assert(data);
    
    height = stack_height(stack);
    for(i = 0; i < height; i++)
    {
        if(compar(data, stack->data + i * stack->size) == 0)
            return true;
    }
    
    return false;
}

/**
 * @brief Returns a pointer to the address where an element is stored.
 * @param stack Stack.
 * @param position Pointer to the memory address were the data at position `position` is stored.
 * @returns A pointer to the memory address.
 */
void *stack_get_element_ptr(const stack_t const *stack, uint32_t position)
{
    return &(stack->data[position * stack->size]);
}


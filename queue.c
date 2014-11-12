#include "queue.h"

void queue_init(queue_t *queue, uint32_t size)
{
    stack_init(&(queue->in), size);
    stack_init(&(queue->out), size);
}

void queue_destroy(queue_t *queue)
{
    stack_destroy(&(queue->in));
    stack_destroy(&(queue->out));
}

bool queue_isempty(const queue_t const *queue)
{
    return (stack_isempty(&(queue->in)) && stack_isempty(&(queue->out)));
}

void queue_enqueue(queue_t *queue, void *data)
{
    stack_push(&(queue->in), data);
}

bool queue_dequeue(queue_t *queue, void *data)
{
    if(stack_isempty(&(queue->out)))
    {
        uint32_t height;
        while((height = stack_height(&(queue->in))) > 0)
        {
            void *element = stack_get_element_ptr(&(queue->in), height - 1);
            stack_push(&(queue->out), element);
            stack_remove_last(&(queue->in));
        }
    }

    if(stack_isempty(&(queue->out)))
        return false;

    stack_pop(&(queue->out), data);
    return true;
}


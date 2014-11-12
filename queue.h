#ifndef QUEUE_H_INCLUDED
#define QUEUE_H_INCLUDED

#include "stack.h"

struct _queue_s
{
    stack_t in, out;
};
typedef struct _queue_s queue_t;

void queue_init(queue_t *queue, uint32_t size);
void queue_destroy(queue_t *queue);
bool queue_isempty(const queue_t const *queue);
void queue_enqueue(queue_t *queue, void *data);
bool queue_dequeue(queue_t *queue, void *data);

#endif

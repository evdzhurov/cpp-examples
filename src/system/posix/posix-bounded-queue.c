
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct BQueue
{
    int capacity;
    int size;
    int head;
    int tail;
    int* slots;
} BQueue;

static BQueue* queue_init(int capacity)
{
    BQueue* queue = malloc(sizeof(BQueue));
    if (!queue)
        return queue;

    queue->capacity = capacity;
    queue->size = 0;
    queue->head = 0;
    queue->tail = 0;
    queue->slots = calloc(queue->capacity, sizeof(int));

    if (!queue->slots)
    {
        free(queue);
        return NULL;
    }

    return queue;
}

static void queue_destroy(BQueue* queue)
{
    if (!queue)
        return;

    free(queue->slots);
    free(queue);
}

static bool queue_empty(BQueue* queue)
{
    return !queue || queue->size == 0;
}

static bool queue_full(BQueue* queue)
{
    return queue && queue->size == queue->capacity;
}

static void queue_print(const char* text, BQueue* queue)
{
    if (queue)
    {
        printf("%s [", text);
        for (int i = 0; i < queue->capacity; ++i)
        {
            if (i == queue->head && i == queue->tail)
                printf("HT:%d,", queue->slots[i]);
            else if (i == queue->head)
                printf("H:%d,", queue->slots[i]);
            else if (i == queue->tail)
                printf("T:%d,", queue->slots[i]);
            else
                printf("%d,", queue->slots[i]);
        }
        printf("]\n");
    }
}

static bool queue_try_push(BQueue* queue, int val)
{
    if (!queue || queue_full(queue))
        return false;

    queue->slots[queue->tail] = val;
    queue->tail = (queue->tail + 1) % queue->capacity;
    ++queue->size;

    return true;
}

int main()
{
    // Init empty queue
    BQueue* q1 = queue_init(5);
    assert(queue_empty(q1));
    assert(!queue_full(q1));
    queue_print("Empty queue", q1);

    // Try push up to capacity and then fail
    for (int i = 1; i <= 5; ++i)
        assert(queue_try_push(q1, i * i));
    assert(!queue_try_push(q1, -1));
    queue_print("Full queue", q1);

    // Cleanup
    queue_destroy(q1);

    return 0;
}
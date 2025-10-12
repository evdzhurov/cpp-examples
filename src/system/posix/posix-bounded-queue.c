
#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct BQueue
{
    size_t capacity;
    size_t size;
    size_t head;
    size_t tail;
    int* slots;

    pthread_mutex_t mtx;
    pthread_cond_t not_empty_cond; // consumer waits for not empty
    pthread_cond_t not_full_cond;  // producer waits for not full
    bool close;

} BQueue;

static BQueue* queue_init(size_t capacity)
{
    BQueue* queue = malloc(sizeof(BQueue));
    if (!queue)
        return NULL;

    queue->capacity = capacity > 0 ? capacity : 1;
    queue->size = 0;
    queue->head = 0;
    queue->tail = 0;
    queue->close = false;

    int err = pthread_mutex_init(&queue->mtx, NULL);
    if (err != 0)
    {
        printf("pthread_mutex_init err: %s", strerror(err));
        goto cleanup;
    }

    err = pthread_cond_init(&queue->not_empty_cond, NULL);
    if (err != 0)
    {
        printf("not_empty_cond pthread_cond_init err: %s", strerror(err));
        goto cleanup_mtx;
    }

    err = pthread_cond_init(&queue->not_full_cond, NULL);
    if (err != 0)
    {
        printf("not_full_cond pthread_cond_init err: %s", strerror(err));
        goto cleanup_cond_not_empty;
    }

    queue->slots = calloc(queue->capacity, sizeof(int));
    if (!queue->slots)
        goto cleanup_cond_not_full;

    return queue;

cleanup_cond_not_full:
    pthread_cond_destroy(&queue->not_full_cond);
cleanup_cond_not_empty:
    pthread_cond_destroy(&queue->not_empty_cond);
cleanup_mtx:
    pthread_mutex_destroy(&queue->mtx);
cleanup:
    free(queue);
    return NULL;
}

static void queue_drain(BQueue* queue)
{
    if (!queue)
        return;
}

static void queue_destroy(BQueue* queue)
{
    if (!queue)
        return;

    pthread_cond_destroy(&queue->not_full_cond);
    pthread_cond_destroy(&queue->not_empty_cond);
    pthread_mutex_destroy(&queue->mtx);
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

static bool queue_try_pop(BQueue* queue, int* val)
{
    if (!queue || queue_empty(queue))
        return false;

    if (val) // discards the value if the user doesn't want it
        *val = queue->slots[queue->head];
    queue->head = (queue->head + 1) % queue->capacity;
    --queue->size;
    return true;
}

static bool queue_wait_push(BQueue* queue, int val)
{
    return false;
}

static bool queue_wait_pop(BQueue* queue, int* val)
{
    return false;
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

    // Try pop until empty
    printf("pop: ");
    for (;;)
    {
        int val;
        bool res = queue_try_pop(q1, &val);
        if (!res)
        {
            printf("empty!\n");
            break;
        }
        printf("%d,", val);
    }
    queue_print("After pop until empty", q1);

    for (int i = 1; i <= q1->capacity / 2 + 1; ++i)
    {
        assert(queue_try_push(q1, i));
        assert(queue_try_pop(q1, NULL));
    }
    queue_print("After push/pop majority of the capacity", q1);
    assert(queue_empty(q1));

    // Cleanup
    queue_drain(q1);
    queue_destroy(q1);

    return 0;
}
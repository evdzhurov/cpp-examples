
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
    if (!queue)
        return false;

    int err = pthread_mutex_lock(&queue->mtx);
    if (err != 0)
    {
        printf("queue_wait_push pthread_mutex_lock err!\n");
        exit(1);
    }

    while (queue->size == queue->capacity && !queue->close)
    {
        err = pthread_cond_wait(&queue->not_full_cond, &queue->mtx);
        if (err != 0)
        {
            printf("queue_wait_push pthread_cond_wait err!\n");
            exit(1);
        }
    }

    if (queue->close)
    {
        err = pthread_mutex_unlock(&queue->mtx);
        if (err != 0)
        {
            printf("queue_wait_push (closed) pthread_mutex_unlock err!\n");
            exit(1);
        }

        return false;
    }

    queue->slots[queue->tail] = val;
    queue->tail = (queue->tail + 1) % queue->capacity;
    ++queue->size;

    err = pthread_mutex_unlock(&queue->mtx);
    if (err != 0)
    {
        printf("queue_wait_push pthread_mutex_unlock err!\n");
        exit(1);
    }

    err = pthread_cond_signal(&queue->not_empty_cond);
    if (err != 0)
    {
        printf("queue_wait_push pthread_cond_signal err!\n");
        exit(1);
    }

    return true;
}

static bool queue_wait_pop(BQueue* queue, int* val)
{
    if (!queue)
        return false;

    int err = pthread_mutex_lock(&queue->mtx);
    if (err != 0)
    {
        printf("queue_wait_pop pthread_mutex_lock err!\n");
        exit(1);
    }

    while (queue->size == 0 && !queue->close)
    {
        err = pthread_cond_wait(&queue->not_empty_cond, &queue->mtx);
        if (err != 0)
        {
            printf("queue_wait_pop pthread_cond_wait err!\n");
            exit(1);
        }
    }

    if (queue->size == 0) // Implies queue->close == true
    {
        err = pthread_mutex_unlock(&queue->mtx);
        if (err != 0)
        {
            printf("queue_wait_pop (close) pthread_mutex_unlock err!\n");
            exit(1);
        }
        return false;
    }

    if (val) // Discards the value if val == NULL
        *val = queue->slots[queue->head];

    queue->head = (queue->head + 1) % queue->capacity;
    --queue->size;

    err = pthread_mutex_unlock(&queue->mtx);
    if (err != 0)
    {
        printf("queue_wait_pop pthread_mutex_unlock err!\n");
        exit(1);
    }

    err = pthread_cond_signal(&queue->not_full_cond);
    if (err != 0)
    {
        printf("queue_wait_pop pthread_cond_signal err!\n");
        exit(1);
    }

    return true;
}

static void queue_close(BQueue* queue)
{
    if (!queue)
        return;

    int err = pthread_mutex_lock(&queue->mtx);
    if (err != 0)
    {
        printf("queue_close pthread_mutex_lock err!\n");
        exit(1);
    }

    if (queue->close)
    {
        err = pthread_mutex_unlock(&queue->mtx);
        if (err != 0)
        {
            printf("queue_close pthread_mutex_unlock err!\n");
            exit(1);
        }

        return;
    }

    queue->close = true;

    err = pthread_mutex_unlock(&queue->mtx);
    if (err != 0)
    {
        printf("queue_close pthread_mutex_unlock err!\n");
        exit(1);
    }

    err = pthread_cond_broadcast(&queue->not_empty_cond);
    if (err != 0)
    {
        printf("queue_close (not_empty_cond) pthread_cond_broadcast err!\n");
        exit(1);
    }

    err = pthread_cond_broadcast(&queue->not_full_cond);
    if (err != 0)
    {
        printf("queue_close (not_full_cond) pthread_cond_broadcast err!\n");
        exit(1);
    }
}

static const size_t PROD_COUNT = 1500;
static const size_t PROD_N = 10;
static const size_t CONS_N = 5;

void* produce(void* arg)
{
    BQueue* queue = arg;
    if (!queue)
        return NULL;

    for (size_t i = 0; i < PROD_COUNT; ++i)
    {
        if (!queue_wait_push(queue, 1))
        {
            return NULL;
        }
    }

    return NULL;
}

void* consume(void* arg)
{
    BQueue* queue = arg;
    if (!queue)
        return NULL;

    size_t count = 0;
    for (;;)
    {
        int val;
        if (!queue_wait_pop(queue, &val))
            break;
        count += val;
    }

    return (void*)count;
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

    pthread_t prod[PROD_N];
    pthread_t cons[CONS_N];

    for (size_t i = 0; i < PROD_N; ++i)
    {
        int err = pthread_create(&prod[i], NULL, produce, q1);
        if (err != 0)
        {
            printf("pthread_create (producer) err!\n");
            exit(1);
        }
    }
    for (size_t i = 0; i < CONS_N; ++i)
    {

        int err = pthread_create(&cons[i], NULL, consume, q1);
        if (err != 0)
        {
            printf("pthread_create (consumer) err!\n");
            exit(1);
        }
    }

    // Cleanup
    printf("Waiting for producers...\n");
    for (size_t i = 0; i < PROD_N; ++i)
    {
        int err = pthread_join(prod[i], NULL);
        if (err != 0)
        {
            printf("pthread_join (producer) err!\n");
            exit(1);
        }
    }

    printf("Closing queue...\n");
    queue_close(q1);

    printf("Waiting for consumers...\n");
    size_t total_count = 0;
    for (size_t i = 0; i < CONS_N; ++i)
    {
        void* ret;
        const int err = pthread_join(cons[i], &ret);
        if (err != 0)
        {
            printf("pthread_create (consumer) err!\n");
            exit(1);
        }

        const size_t count = (size_t)ret;
        total_count += count;
    }

    printf("Total count expected: %lu\n", (unsigned long)(PROD_COUNT * PROD_N));
    printf("Total count consumed: %lu\n", (unsigned long)total_count);

    printf("Destroying queue...\n");
    queue_destroy(q1);

    printf("Done.\n");
    return 0;
}
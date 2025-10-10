#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
static int avail = 0;

static void* producer(void* arg)
{
    const int count = *(const int*)arg;
    for (int i = 0; i < count; ++i)
    {
        sleep(1);

        int err = pthread_mutex_lock(&mtx);
        if (err != 0)
        {
            printf("producer: pthread_mutex_lock error!\n");
            exit(1);
        }

        avail++;

        err = pthread_mutex_unlock(&mtx);
        if (err != 0)
        {
            printf("producer: pthread_mutex_unlock error!\n");
            exit(1);
        }

        err = pthread_cond_signal(&cond);
        if (err != 0)
        {
            printf("producer: pthread_cond_signal error!\n");
            exit(1);
        }
    }

    return NULL;
}

int main(int argc, char* arv[])
{
    time_t t = time(NULL);

    int totalRequired = 0;
    int producer_counts[] = {10, 3, 7, 1, 1, 1};

    const int numProd = sizeof(producer_counts) / sizeof(producer_counts[0]);
    for (int i = 0; i < numProd; ++i)
    {
        totalRequired += producer_counts[i];

        pthread_t tid;
        int err = pthread_create(&tid, NULL, producer, &producer_counts[i]);
        if (err != 0)
        {
            printf("producer: pthread_create error!\n");
            exit(1);
        }
    }

    int numConsumed = 0;
    for (;;)
    {
        int err = pthread_mutex_lock(&mtx);
        if (err != 0)
        {
            printf("consumer: pthread lock error!\n");
            exit(1);
        }

        while (avail == 0)
        {
            err = pthread_cond_wait(&cond, &mtx);
            if (err != 0)
            {
                printf("consumer: pthread_cond_wait error!\n");
                exit(1);
            }
        }

        numConsumed += avail;
        avail = 0;
        printf("T=%ld: numConsumed=%d\n", (long)(time(NULL) - t), numConsumed);

        err = pthread_mutex_unlock(&mtx);
        if (err != 0)
        {
            printf("consumer: pthread unlock error!\n");
            exit(1);
        }

        if (numConsumed >= totalRequired)
            break;
    }

    return 0;
}
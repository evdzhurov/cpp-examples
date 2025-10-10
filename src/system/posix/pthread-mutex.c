#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int global = 0;
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

static void* threadFunc(void* arg)
{
    int loops = *((int*)arg);
    int local = 0;
    char strerr_buf[128];
    for (int i = 0; i < loops; ++i)
    {
        int err = pthread_mutex_lock(&mtx);
        if (err != 0)
        {
            strerror_r(err, strerr_buf, sizeof(strerr_buf));
            printf("pthread_mutex_lock() err: %s\n", strerr_buf);
            exit(1);
        }

        local = global;
        ++local;
        global = local;

        err = pthread_mutex_unlock(&mtx);
        if (err != 0)
        {
            strerror_r(err, strerr_buf, sizeof(strerr_buf));
            printf("pthread_mutex_unlock() err: %s\n", strerr_buf);
            exit(1);
        }
    }

    return NULL;
}

int main(int argc, char* argv[])
{
    int loops = (argc > 1) ? atoi(argv[1]) : 1000000;

    pthread_t t1, t2;
    int err = pthread_create(&t1, NULL, threadFunc, &loops);
    if (err != 0)
    {
        printf("pthread_create(t1) err: %s", strerror(err));
        return 1;
    }

    err = pthread_create(&t2, NULL, threadFunc, &loops);
    if (err != 0)
    {
        printf("pthread_create(t2) err: %s", strerror(err));
        return 1;
    }

    err = pthread_join(t1, NULL);
    if (err != 0)
    {
        printf("pthread_join(t1) err: %s", strerror(err));
        return 1;
    }

    err = pthread_join(t2, NULL);
    if (err != 0)
    {
        printf("pthread_join(t2) err: %s", strerror(err));
        return 1;
    }

    printf("global = %d\n", global);

    return 0;
}
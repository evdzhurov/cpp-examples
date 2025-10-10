#include <pthread.h>
#include <stdio.h>
#include <string.h>

void* thread_func(void* arg)
{
    char* s = (char*)arg;
    printf("thread_func: %s\n", s);
    return (void*)strlen(s);
}

int main(int argc, char* argv[])
{
    pthread_t t1;
    int s = pthread_create(&t1, NULL, thread_func, "Hello, World!");
    if (s != 0)
    {
        printf("pthread_create err: %s\n", strerror(s));
        return 1;
    }

    printf("Message from main()\n");

    void* ret;
    s = pthread_join(t1, &ret);
    if (s != 0)
    {
        printf("pthread_join err: %s\n", strerror(s));
        return 1;
    }

    printf("Thread returned: %ld\n", (long)ret);

    return 0;
}
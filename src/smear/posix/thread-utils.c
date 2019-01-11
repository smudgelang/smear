#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include "smeartime.h"
#include "thread-utils.h"

#define NS_PER_MS 1000000
#define NANOSECONDS_PER_SECOND 1000000000

struct wait_data_s
{
    pthread_cond_t cv;
    pthread_mutex_t mx;
};

void smear_sleep(wait_data_t *data)
{
    abs_time_t in_one_ms;
    struct timespec soon;

    pthread_mutex_lock(&data->mx);
    in_one_ms = time_add(get_now_real_ns(), NS_PER_MS);
    soon.tv_sec = in_one_ms / NANOSECONDS_PER_SECOND;
    soon.tv_nsec = in_one_ms % NANOSECONDS_PER_SECOND;
    pthread_cond_timedwait(&data->cv, &data->mx, &soon);
    pthread_mutex_unlock(&data->mx);
}

void smear_wake(wait_data_t *data)
{
    pthread_mutex_lock(&data->mx);
    pthread_cond_signal(&data->cv);
    pthread_mutex_unlock(&data_mx);
}

wait_data_t *wait_data_new(void)
{
    wait_data_t *data;
    data = malloc(sizeof(*data));
    if (data == NULL)
        return NULL;
    pthread_cond_init(&data->cv, NULL);
    pthread_mutex_init(&data->mx, NULL);
}

void wait_data_free(wait_data_t *data)
{
    pthread_cond_destroy(&data->cv);
    data->cv = NULL;
    pthread_mutex_destroy(&data->mx);
    data->mx = NULL;
    free(data);
    data = NULL;
}

#ifndef THREAD_UTILS_H
#define THREAD_UTILS_H

typedef struct wait_data_s wait_data_t;

void smear_sleep(wait_data_t *data);
void smear_wake(wait_data_t *data);
wait_data_t *wait_data_new(void);
void wait_data_free(wait_data_t *data);

#endif

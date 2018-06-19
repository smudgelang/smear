#define _GNU_SOURCE // for reallocarray
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <pthread.h>
#include "queue.h"

#define INITIAL_SIZE 4 // Must be a power of 2

struct queue_s
{
    size_t write;
    size_t read;
    size_t capacity; // Must always be a power of 2
    pthread_mutex_t mutex;
    pthread_cond_t empty;
    const void **ring;
};

static size_t size_LH(const queue_t *q)
{
    return q->write - q->read;
}

static size_t real_index(size_t large, size_t capacity)
{
    size_t mask;

    // Because capacity is always a power of 2:
    assert((capacity & (capacity - 1)) == 0);
    // n % capacity == n & (capacity - 1)
    mask = capacity - 1;
    return large & mask;
}

static void grow_q(queue_t *q)
{
    void *bigger;
    size_t real_r, real_w;

    bigger = reallocarray(q->ring, q->capacity * 2, sizeof(*q->ring));
    if (bigger == NULL)
    {
        // It's fine, maybe we're out of memory. The enqueue
        // operation will fail, but we don't need to crash here.
        return;
    }

    real_r = real_index(q->read, q->capacity);
    real_w = real_index(q->write, q->capacity);
    if (real_w <= real_r)
    {
        size_t bytes;

        // Move indices [0, real_w) up to capacity.
        bytes = real_w * sizeof(q->ring[0]);
        memmove(&q->ring[q->capacity], q->ring, bytes);
    }

    while (q->read > q->capacity)
    {
        q->read -= q->capacity;
        q->write -= q->capacity;
    }
    if (q->write == q->read)
    {
        q->write = q->capacity;
    }
    q->capacity *= 2;
    q->ring = bigger;
    return;
}

static void settle(queue_t *q, bool inserting)
{
    if (q->read == q->write)
    {
        q->read = 0;
        q->write = 0;
        return;
    }

    if (q->read > q->capacity)
    {
        q->read = real_index(q->read, q->capacity);
        q->write = real_index(q->write, q->capacity);
    }

    // Resize queue when necessary/appropriate.
    if (size_LH(q) == q->capacity && inserting)
    {
        grow_q(q);
    }
}

queue_t *newq(void)
{
    queue_t *q;

    q = malloc(sizeof(*q));
    q->read = 0;
    q->write = 0;
    q->capacity = INITIAL_SIZE;
    pthread_mutex_init(&q->mutex, NULL);
    q->ring = calloc(q->capacity, sizeof(*q->ring));
    pthread_cond_init(&q->empty, NULL);
    return q;
}

void freeq(queue_t *queue)
{
    assert(pthread_mutex_lock(&queue->mutex) == 0);
    pthread_mutex_unlock(&queue->mutex);
    pthread_mutex_destroy(&queue->mutex);
    free(queue->ring);
    free(queue);
    queue = NULL;
}

bool enqueue(queue_t *q, const void *value)
{
    size_t index;
    bool success;

    success = false;
    if (pthread_mutex_lock(&q->mutex) != 0)
        goto done;

    settle(q, true);
    if (size_LH(q) == q->capacity)
        goto fail;
    index = real_index(q->write, q->capacity);
    q->ring[index] = value;
    q->write++;
    success = true;
fail:
    pthread_mutex_unlock(&q->mutex);
done:
    return success;
}

bool dequeue(queue_t *q, const void **value)
{
    size_t index;
    bool success;

    success = false;
    if (pthread_mutex_lock(&q->mutex) != 0)
        goto done;
    settle(q, false);
    if (q->write == q->read)
        goto fail;
    index = real_index(q->read, q->capacity);
    *value = q->ring[index];
    q->read++;
    if (size_LH(q) == 0)
        pthread_cond_broadcast(&q->empty);
    success = true;
fail:
    pthread_mutex_unlock(&q->mutex);
done:
    return success;
}

size_t size(queue_t *q)
{
	size_t s;

	if (pthread_mutex_lock(&q->mutex) != 0)
	{
		perror("Failed to get queue mutex.");
		exit(-1);
	}
	s = size_LH(q);
	pthread_mutex_unlock(&q->mutex);
	return s;
}

void wait_empty(queue_t *q)
{
    pthread_mutex_lock(&q->mutex);
    while(size_LH(q) != 0)
    {
        pthread_cond_wait(&q->empty, &q->mutex);
    }
    pthread_mutex_unlock(&q->mutex);
    return;
}

#if 0
int main(void)
{
    queue_t *q;
    const void *val;

    q = newq();
    assert(enqueue(q, (const void *)0x1111111100000000)); // 1
    assert(enqueue(q, (const void *)0x2222222211111111)); // 2
    assert(dequeue(q, &val));                             // 1
    assert(val == (void *)0x1111111100000000);
    assert(enqueue(q, (const void *)0x3333333322222222)); // 2
    assert(dequeue(q, &val));                             // 1
    assert(val == (void *)0x2222222211111111);
    assert(enqueue(q, (const void *)0x4444444433333333)); // 2
    assert(enqueue(q, (const void *)0x5555555544444444)); // 3
    assert(enqueue(q, (const void *)0x6666666655555555)); // 4
    assert(enqueue(q, (const void *)0x7777777766666666)); // 5
    assert(enqueue(q, (const void *)0x8888888877777777)); // 6
    assert(enqueue(q, (const void *)0x9999999988888888)); // 7
    assert(dequeue(q, &val)); // 6
    assert(val == (void *)0x3333333322222222);
    assert(dequeue(q, &val)); // 5
    assert(val == (void *)0x4444444433333333);
    assert(dequeue(q, &val)); // 4
    assert(val == (void *)0x5555555544444444);
    assert(dequeue(q, &val)); // 3
    assert(val == (void *)0x6666666655555555);
    assert(dequeue(q, &val)); // 2
    assert(val == (void *)0x7777777766666666);
    assert(dequeue(q, &val)); // 1
    assert(val == (void *)0x8888888877777777);
    assert(dequeue(q, &val)); // 0
    assert(val == (void *)0x9999999988888888);

    return q->capacity;
}
#endif


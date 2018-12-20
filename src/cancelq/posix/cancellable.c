#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <inttypes.h>
#include <pthread.h>

#include "platform.h"
#include "cancellable.h"

#ifndef HEAP_CHECK
#define HEAP_CHECK true // Set to false to save oodles of execution time.
#endif

#define BASE_HEAP_SIZE 25
#define INITIAL_ID_COUNT 16

// Normal events that can't be cancelled
typedef struct
{
    const void *event;
} event_t;

// Events that can be cancelled, which includes events whose sending
// can be delayed
typedef struct
{
    event_t event;
    cancellable_id_t id;
    abs_time_t delivery_time;
} cancellable_t;

typedef cancellable_t heap_data_t;
#include "heap.c" // Note the .c

typedef enum
{
    ID_UNUSED = 0,
    ID_DELIVERED,
    ID_WAITING
} id_state_t;

struct event_queue_s
{
    heap_t heap;
    // Current size of the array of IDs
    cancellable_id_t idsize;

    // Array of IDs for cancellation. The ID is an index into this array.
    id_state_t *ids;

    pthread_mutex_t lock;
    pthread_cond_t empty;
};

UNUSED static void print_queue(event_queue_t *q)
{
    print_heap(&q->heap);
}

static bool empty_LH(const event_queue_t *q)
{
    return heap_empty(&q->heap);
}

static cancellable_id_t new_id(event_queue_t *q)
{
    id_state_t *new_ids;
    cancellable_id_t new_size;
    cancellable_id_t id;

    for (id = 0; id < q->idsize; id++)
    {
        if (q->ids[id] == ID_UNUSED)
        {
            q->ids[id] = ID_WAITING;
            return id;
        }
    }

    // We're out of IDs. Double the size of the array for now.
    new_size = q->idsize * 2;
    new_ids = realloc(q->ids, sizeof(id_state_t) * new_size);
    if (new_ids == NULL)
        return NOT_CANCELLABLE;
    q->ids = new_ids;

    // Depends on the fact that we're doubling.
    memset(&q->ids[id], ID_UNUSED, sizeof(q->ids[id]) * q->idsize);

    q->idsize = new_size;

    q->ids[id] = ID_WAITING;
    return id;
}

static bool check_queue(const event_queue_t *q)
{
    return check_heap(&q->heap);
}

event_queue_t *eq_new(void)
{
    event_queue_t *q;

    q = malloc(sizeof(*q));
    if (q == NULL)
        return q;
    newheap(&q->heap, BASE_HEAP_SIZE);
    q->idsize = INITIAL_ID_COUNT;

    if (!check_heap(&q->heap))
    {
        free(q);
        return NULL;
    }

    pthread_mutex_init(&q->lock, NULL);
    pthread_cond_init(&q->empty, NULL);

    // Initializes to ID_UNUSED.
    q->ids = calloc(INITIAL_ID_COUNT, sizeof(*q->ids));
    return q;
}

bool eq_free(event_queue_t *q)
{
    if (pthread_mutex_lock(&q->lock) != 0)
        return false;

    if (!empty_LH(q))
    {
        pthread_mutex_unlock(&q->lock);
        return false;
    }

    heap_free(&q->heap);
    memset(q->ids, 0, q->idsize * sizeof(q->ids[0]));
    free(q->ids);
    q->ids = NULL;

    pthread_mutex_unlock(&q->lock);
    pthread_mutex_destroy(&q->lock);

    memset(q, 0, sizeof(*q));
    free(q);
    return true;
}

cancellable_id_t eq_schedule(event_queue_t *q, const void *event,
                             abs_time_t time)
{
    cancellable_t wrapper;
    cancellable_id_t id;

    id = SCHEDULE_FAIL;
    if (pthread_mutex_lock(&q->lock) != 0)
        goto done;

    wrapper.event.event = event;
    id = new_id(q);

    if (id == NOT_CANCELLABLE)
    {
        id = SCHEDULE_FAIL;
        goto fail;
    }

    wrapper.id = id;
    wrapper.delivery_time = time;
    if (!heap_insert(&q->heap, &wrapper))
    {
        q->ids[id] = ID_UNUSED;
        id = SCHEDULE_FAIL;
        goto fail;
    }

    if (HEAP_CHECK)
        assert(check_queue(q));

fail:
    pthread_mutex_unlock(&q->lock);
done:
    return id;
}

bool eq_post(event_queue_t *q, const void *event, abs_time_t time)
{
    cancellable_t wrapper;
    bool success;

    success = false;
    if (pthread_mutex_lock(&q->lock) != 0)
        goto done;

    wrapper.event.event = event;
    wrapper.id = NOT_CANCELLABLE;
    wrapper.delivery_time = time;
    if (!heap_insert(&q->heap, &wrapper))
        goto fail;

    if (HEAP_CHECK)
        assert(check_queue(q));

    success = true;
fail:
    pthread_mutex_unlock(&q->lock);
done:
    return success;
}

void *eq_next_event(event_queue_t *q, abs_time_t time)
{
    cancellable_t *next;
    const void *event;

    event = NULL;
    next = NULL;
    if (pthread_mutex_lock(&q->lock) != 0)
        goto done;

    next = heap_peek(&q->heap, time);
    if (next == NULL)
        goto fail;

    event = next->event.event;
    if (next->id != NOT_CANCELLABLE)
    {
        assert(q->ids[next->id] == ID_WAITING);
        q->ids[next->id] = ID_DELIVERED;
    }

    heap_rm(&q->heap, 1);
fail:
    if (empty_LH(q))
        pthread_cond_broadcast(&q->empty);

    pthread_mutex_unlock(&q->lock);
done:
    return (void *)event;
}

bool eq_empty(event_queue_t *q)
{
    bool empty;

    empty = false;
    if (pthread_mutex_lock(&q->lock) == 0)
    {
        empty = empty_LH(q);
        pthread_mutex_unlock(&q->lock);
    }

    return empty;
}

static cancellation_status_t release_LH(event_queue_t *q, cancellable_id_t id)
{
    if (q->idsize < id)
    {
        return FAIL_NO_SUCH_ID;
    }
    switch (q->ids[id])
    {
    case ID_UNUSED:
        return FAIL_NO_SUCH_ID;
    case ID_WAITING:
        return FAIL_NOT_RUN;
    case ID_DELIVERED:
        break;
    }
    q->ids[id] = ID_UNUSED;
    return SUCCESS;
}

static cancellation_status_t cancel_LH(event_queue_t *q, cancellable_id_t id,
                                       void **event)
{
    size_t idx;

    if (q->idsize < id)
    {
        return FAIL_NO_SUCH_ID;
    }

    switch (q->ids[id])
    {
    case ID_UNUSED:
        return FAIL_NO_SUCH_ID;
    case ID_WAITING:
        break;
    case ID_DELIVERED:
        return FAIL_ALREADY_RUN;
    }

    // This is pretty deep into the heap's data structure, but heaps
    // don't do this.
    for (idx = 1; idx < q->heap.nextidx; idx++)
    {
        if (q->heap.data[idx].id == id)
        {
            *event = (void *)q->heap.data[idx].event.event;
            heap_rm(&q->heap, idx);
            q->ids[id] = ID_UNUSED;
            return SUCCESS;
        }
    }
    assert(false); // This should never ever happen; it's waiting but corrupted.
    return FAIL_NO_SUCH_ID; // Appease compilers that don't know about assert.
}

cancellation_status_t eq_cancel(event_queue_t *q, cancellable_id_t id,
                                void **event)
{
    cancellation_status_t status;

    status = FAIL_LOCKING;
    *event = NULL;
    if (pthread_mutex_lock(&q->lock) == 0)
    {
        status = cancel_LH(q, id, event);
        pthread_mutex_unlock(&q->lock);
    }

    return status;
}

cancellation_status_t eq_cancel_or_release(event_queue_t *q,
                                           cancellable_id_t id,
                                           void **event)
{
    cancellation_status_t status;

    status = FAIL_LOCKING;
    *event = NULL;
    if (pthread_mutex_lock(&q->lock) != 0)
        goto end;
    if (q->idsize < id)
    {
        status = FAIL_NO_SUCH_ID;
        goto unlock;
    }

    switch (q->ids[id])
    {
    case ID_UNUSED:
        status = FAIL_NO_SUCH_ID;
        break;
    case ID_WAITING:
        status = cancel_LH(q, id, event);
        break;
    case ID_DELIVERED:
        status = release_LH(q, id);
        break;
    }

unlock:
    pthread_mutex_unlock(&q->lock);
end:
    return status;
}

bool eq_validate(event_queue_t *q)
{
    bool status;

    status = false;
    if (pthread_mutex_lock(&q->lock) == 0)
    {
        status = check_queue(q);
        pthread_mutex_unlock(&q->lock);
    }
    return status;
}

cancellation_status_t eq_release(event_queue_t *q, cancellable_id_t id)
{
    cancellation_status_t status;

    status = FAIL_LOCKING;
    if (pthread_mutex_lock(&q->lock) == 0)
    {
        status = release_LH(q, id);
        pthread_mutex_unlock(&q->lock);
    }

    return status;
}

void eq_wait_empty(event_queue_t *q)
{
    pthread_mutex_lock(&q->lock);
    while(!empty_LH(q))
    {
        pthread_cond_wait(&q->empty, &q->lock);
    }
    pthread_mutex_unlock(&q->lock);
    return;
}

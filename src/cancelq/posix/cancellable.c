#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <inttypes.h>
#include <pthread.h>

#include "cancellable.h"

#define HEAP_CHECK true // Set to false to save oodles of execution time.

#define BASE_HEAP_SIZE 25
#define INITIAL_ID_COUNT 16

#define UNUSED __attribute__((unused))

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

typedef enum
{
    ID_UNUSED = 0,
    ID_DELIVERED,
    ID_WAITING
} id_state_t;

struct event_queue_s
{
    // Size includes the 1 empty slot at the beginning. We could play
    // games with pointers to save that sizeof(cancellable_t)
    // but...not until it's working.
    size_t heapsize;

    // Index of the next element to be added to the heap.
    size_t heapidx;
    cancellable_t *minheap;

    // Current size of the array of IDs
    cancellable_id_t idsize;

    // Array of IDs for cancellation. The ID is an index into this array.
    id_state_t *ids;

    pthread_mutex_t lock;
    pthread_cond_t empty;
};

UNUSED static bool is_pwr_of_2(size_t num)
{
    if (num == 0)
        return false;

    while (num % 2 == 0)
    {
        num /= 2;
    }
    return num == 1;
}

UNUSED static void print_heap(event_queue_t *q)
{
    cancellable_t *heap;

    heap = q->minheap;

    for (size_t idx = 1; idx < q->heapidx; idx++)
    {
        if (is_pwr_of_2(idx))
            printf("\n");
        printf(" %"PRIu64" ", heap[idx].delivery_time);
    }
    printf("\n");
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

static size_t parent(size_t idx)
{
    return idx / 2; // floor division on purpose!
}

static size_t left(size_t idx)
{
    return idx * 2;
}

static size_t right(size_t idx)
{
    return idx * 2 + 1;
}

static void swap(cancellable_t *heap, size_t a, size_t b)
{
    cancellable_t temp;

    memcpy(&temp, &heap[a], sizeof(temp));
    memcpy(&heap[a], &heap[b], sizeof(temp));
    memcpy(&heap[b], &temp, sizeof(temp));
}

static bool empty_LH(const event_queue_t *q)
{
    return q->heapidx == 1;
}

static bool check_heap(event_queue_t *q)
{
    cancellable_t *heap;

    heap = q->minheap;
    for (size_t i = 1; i < q->heapidx; i++)
    {
        if (left(i) >= q->heapidx)
            continue;
        if (heap[i].delivery_time > heap[left(i)].delivery_time)
            return false;
        if (right(i) >= q->heapidx)
            continue;
        if (heap[i].delivery_time > heap[right(i)].delivery_time)
            return false;
    }
    return true;
}

static void postins_reheap(cancellable_t *heap, size_t idx)
{
    if (idx <= 1)
        return;
    if (heap[idx].delivery_time < heap[parent(idx)].delivery_time)
    {
        swap(heap, idx, parent(idx));
        postins_reheap(heap, parent(idx));
    }
}

static void reheap(cancellable_t *heap, size_t last_idx)
{
    for (size_t idx = 1; idx <= last_idx; idx++)
    {
        if (left(idx) <= last_idx &&
            heap[idx].delivery_time > heap[left(idx)].delivery_time)
        {
            swap(heap, idx, left(idx));
        }
        if (right(idx) <= last_idx &&
            heap[idx].delivery_time > heap[right(idx)].delivery_time)
        {
            swap(heap, idx, right(idx));
        }
    }
}

static bool grow_heap(event_queue_t *q)
{
    cancellable_t *new_heap;
    size_t new_size;

    new_size = q->heapsize * 2;
    new_heap = realloc(q->minheap, new_size * sizeof(cancellable_t));
    if (new_heap == NULL)
        return false;
    q->minheap = new_heap;
    q->heapsize = new_size;
    return true;
}

static void rm(event_queue_t *q, size_t idx)
{
    q->heapidx--;
    swap(q->minheap, q->heapidx, idx);
    reheap(q->minheap, q->heapidx - 1);
    if (HEAP_CHECK)
        assert(check_heap(q));
    memset(&q->minheap[q->heapidx], 0, sizeof(q->minheap[q->heapidx]));
}

event_queue_t *eq_new(void)
{
    event_queue_t *q;

    q = malloc(sizeof(*q));
    if (q == NULL)
        return q;
    q->minheap = calloc(BASE_HEAP_SIZE, sizeof(*q->minheap));
    q->heapsize = BASE_HEAP_SIZE;
    q->heapidx = 1;
    q->idsize = INITIAL_ID_COUNT;

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

    pthread_mutex_unlock(&q->lock);
    pthread_mutex_destroy(&q->lock);

    free(q->minheap);
    q->minheap = NULL;
    free(q->ids);
    q->ids = NULL;
    free(q);
    return true;
}

cancellable_id_t eq_schedule(event_queue_t *q, const void *event,
                             abs_time_t time)
{
    cancellable_t *wrapper;
    cancellable_id_t id;

    id = SCHEDULE_FAIL;
    if (pthread_mutex_lock(&q->lock) != 0)
        goto done;
    
    if (q->heapidx == q->heapsize && !grow_heap(q))
        goto fail;

    wrapper = &q->minheap[q->heapidx];
    wrapper->event.event = event;
    id = new_id(q);

    // id can be NOT_CANCELLABLE if the id allocation failed, but the
    // event still gets scheduled.
    
    wrapper->id = id;
    wrapper->delivery_time = time;
    postins_reheap(q->minheap, q->heapidx);
    q->heapidx++;
    if (HEAP_CHECK)
        assert(check_heap(q));

    if (empty_LH(q))
        pthread_cond_broadcast(&q->empty);

fail:
    pthread_mutex_unlock(&q->lock);
done:
    return id;
}

bool eq_post(event_queue_t *q, const void *event, abs_time_t time)
{
    cancellable_t *wrapper;
    bool success;

    success = false;
    if (pthread_mutex_lock(&q->lock) != 0)
        goto done;

    if (q->heapidx == q->heapsize && !grow_heap(q))
        goto fail;

    wrapper = &q->minheap[q->heapidx];
    wrapper->event.event = event;
    wrapper->id = NOT_CANCELLABLE;
    wrapper->delivery_time = time;
    postins_reheap(q->minheap, q->heapidx);
    q->heapidx++;
    if (HEAP_CHECK)
        assert(check_heap(q));

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
    
    if (q->heapidx == 1) // Empty
        goto fail;

    next = &q->minheap[1];

    if (next->delivery_time > time) // Not time yet
        goto fail;

    event = next->event.event;
    if (next->id != NOT_CANCELLABLE)
    {
        assert(q->ids[next->id] == ID_WAITING);
        q->ids[next->id] = ID_DELIVERED;
    }
    rm(q, 1);
    if (empty_LH(q))
        pthread_cond_broadcast(&q->empty);

fail:
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

    for (idx = 1; idx < q->heapidx; idx++)
    {
        if (q->minheap[idx].id == id)
        {
            *event = (void *)q->minheap[idx].event.event;
            rm(q, idx);
            q->ids[id] = ID_UNUSED;
            return SUCCESS;
        }
    }
    assert(false); // This should never ever happen; it's waiting but corrupted.
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
        status = check_heap(q);
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

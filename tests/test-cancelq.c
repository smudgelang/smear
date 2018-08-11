#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <pthread.h>
#include <sched.h>
#include "cancellable.h"

#define COUNT 32
#define PARANOID false

void test_fill_then_cancel_all(void)
{
    event_queue_t *q;
    cancellable_id_t ids[COUNT];
    intptr_t e;

    q = eq_new();
    assert(eq_empty(q));
    assert(eq_validate(q));

    for (intptr_t i = 0; i < COUNT; i++)
    {
        ids[i] = eq_schedule(q, (void *)i, i*2);
        if (PARANOID)
            assert(eq_validate(q));
        assert(ids[i] != NOT_CANCELLABLE);
        assert(!eq_empty(q));
        assert(eq_release(q, ids[i]) == FAIL_NOT_RUN);
        if (PARANOID)
            assert(eq_validate(q));
    }

    assert(eq_validate(q));
    for (intptr_t i = COUNT - 1; i >= 0; i--)
    {
        assert(!eq_empty(q));
        if (i & 1)
        {
            assert(eq_cancel(q, ids[i], (void **)&e) == SUCCESS);
            if (PARANOID)
                assert(eq_validate(q));
            assert(e == i);
            assert(eq_cancel_or_release(q, ids[i],(void **)&e) == FAIL_NO_SUCH_ID);
            if (PARANOID)
                assert(eq_validate(q));
            assert(e == (intptr_t)NULL);
        }
        else
        {
            assert(eq_cancel_or_release(q, ids[i], (void **)&e) == SUCCESS);
            if (PARANOID)
                assert(eq_validate(q));
            assert(e == i);
            assert(eq_cancel(q, ids[i], (void **)&e) == FAIL_NO_SUCH_ID);
            if (PARANOID)
                assert(eq_validate(q));
            assert(e == (intptr_t)NULL);
        }
    }

    assert(eq_validate(q));
    assert(eq_empty(q));
    assert(eq_next_event(q, 0) == NULL);
    assert(eq_free(q));
    q = NULL;
}

void test_cancel_some_drain_some(void)
{
    event_queue_t *q;
    cancellable_id_t ids[COUNT];
    void *e;

    q = eq_new();
    assert(eq_empty(q));
    assert(eq_validate(q));

    for (intptr_t i = COUNT - 1; i >= 0; i--)
    {
        ids[i] = eq_schedule(q, (void *)i, i*2);
        if (PARANOID)
            assert(eq_validate(q));
        assert(ids[i] != NOT_CANCELLABLE);
        assert(!eq_empty(q));
        assert(eq_release(q, ids[i]) == FAIL_NOT_RUN);
        if (PARANOID)
            assert(eq_validate(q));
    }

    for (intptr_t i = 0; i < COUNT; i++)
    {
        if ((i & 1) == 0)
        {
            assert(!eq_empty(q));
            e = eq_next_event(q, 1000);
            if (PARANOID)
                assert(eq_validate(q));
            assert(e == (void *)i);
            assert(eq_cancel(q, ids[i], &e) == FAIL_ALREADY_RUN);
            if (PARANOID)
                assert(eq_validate(q));
            assert(e == NULL);
            assert(eq_release(q, ids[i]) == SUCCESS);
            if (PARANOID)
                assert(eq_validate(q));
        }
        else
        {
            assert(!eq_empty(q));
            assert(eq_cancel(q, ids[i], &e) == SUCCESS);
            if (PARANOID)
                assert(eq_validate(q));
            assert(e == (void *)i);
            assert(eq_cancel(q, ids[i], &e) == FAIL_NO_SUCH_ID);
            assert(e == NULL);
            e++;
            assert(eq_cancel_or_release(q, ids[i], &e) == FAIL_NO_SUCH_ID);
            assert(e == NULL);
            if (PARANOID)
                assert(eq_validate(q));
        }
    }

    assert(eq_empty(q));
    assert(eq_next_event(q, 0) == NULL);
    assert(eq_free(q));
    q = NULL;
}

void test_fill_then_drain_all(void)
{
    event_queue_t *q;
    cancellable_id_t ids[COUNT];
    void *e;

    q = eq_new();
    assert(eq_empty(q));
    assert(eq_validate(q));

    for (intptr_t i = COUNT - 1; i >= 0; i--)
    {
        ids[i] = eq_schedule(q, (void *)i, i*2);
        if (PARANOID)
            assert(eq_validate(q));
        assert(ids[i] != NOT_CANCELLABLE);
        assert(!eq_empty(q));
        assert(eq_release(q, ids[i]) == FAIL_NOT_RUN);
        if (PARANOID)
            assert(eq_validate(q));
    }

    for (intptr_t i = 0; i < COUNT; i++)
    {
        assert(!eq_empty(q));
        e = eq_next_event(q, 1000);
        if (PARANOID)
            assert(eq_validate(q));
        assert(e == (void *)i);
        assert(eq_cancel(q, ids[i], &e) == FAIL_ALREADY_RUN);
        if (PARANOID)
            assert(eq_validate(q));
        assert(e == NULL);
        assert(eq_release(q, ids[i]) == SUCCESS);
        if (PARANOID)
            assert(eq_validate(q));
    }

    assert(eq_empty(q));
    assert(eq_validate(q));
    assert(eq_free(q));
    q = NULL;
}

void test_not_cancellable(void)
{
    event_queue_t *q;
    void *e;

    q = eq_new();
    assert(eq_empty(q));
    assert(eq_validate(q));

    for (intptr_t i = 0; i < 0x10000; i++)
    {
        assert(eq_post(q, (void *)i, i));
        assert(!eq_empty(q));
        if (PARANOID)
            assert(eq_validate(q));
    }

    assert(eq_validate(q));

    for (intptr_t i = 0; i < 0x10000; i++)
    {
        e = eq_next_event(q, 0xffffffff);
        assert((intptr_t)e == i);
        if (PARANOID)
            assert(eq_validate(q));
    }
    assert(eq_empty(q));
    assert(eq_validate(q));
    assert(eq_free(q));
    q = NULL;
}

static void *insert1(void *q)
{
    intptr_t i;
    cancellable_id_t *ids;

    ids = calloc(COUNT, sizeof(ids[0]));
    assert(ids != NULL);
    for (i = 0; i < COUNT; i++)
        ids[i] = NOT_CANCELLABLE;

    for (i = 0; i < COUNT; i+= 2)
    {
        ids[i] = eq_schedule(q, (void *)i, i);
        assert(ids[i] != NOT_CANCELLABLE);
    }
    return ids;
}

static void *insert2(void *q)
{
    intptr_t i;
    cancellable_id_t *ids;

    ids = calloc(COUNT, sizeof(ids[0]));
    assert(ids != NULL);
    for (i = 0; i < COUNT; i++)
        ids[i] = NOT_CANCELLABLE;
    
    for (i = 1; i < COUNT; i+= 2)
    {
        ids[i] = eq_schedule(q, (void *)i, i);
        assert(ids[i] != NOT_CANCELLABLE);
    }
    return ids;
}

static void *wait_for_empty(void *q)
{
    eq_wait_empty(q);
    return NULL;
}

void test_threads(void)
{
    event_queue_t *q;
    pthread_attr_t attr;
    pthread_t t1, t2, waiter;
    intptr_t i;
    void *rv;
    cancellable_id_t *ids;
    void *e;

    q = eq_new();
    assert(q != NULL);
    assert(eq_empty(q));
    pthread_attr_init(&attr);
    pthread_create(&t1, &attr, insert1, q);
    pthread_create(&t2, &attr, insert2, q);
    pthread_create(&waiter, &attr, wait_for_empty, q);
    pthread_attr_destroy(&attr);

    for (i = 0; i < COUNT; i++)
    {
        do
        {
            while (eq_empty(q))
                sched_yield();
            if (PARANOID)
                assert(eq_validate(q));
            e = eq_next_event(q, i);
        } while (e == NULL);

        assert(e == (void *)i);
    }

    pthread_join(waiter, &rv);
    assert(rv == NULL);
    
    pthread_join(t1, (void **)&ids);
    assert(ids != NULL);
    for (i = 0; i < COUNT; i++)
    {
        if (ids[i] == NOT_CANCELLABLE)
            continue;
        assert(eq_release(q, ids[i]) == SUCCESS);
    }
    free(ids);
    ids = NULL;

    pthread_join(t2, (void **)&ids);
    assert(ids != NULL);
    for (i = 0; i < COUNT; i++)
    {
        if (ids[i] == NOT_CANCELLABLE)
            continue;
        assert(eq_cancel_or_release(q, ids[i], &e) == SUCCESS);
        assert(e == NULL);
    }
    free(ids);
    ids = NULL;
    assert(eq_free(q));
}

int main(void)
{
    test_fill_then_cancel_all();
    test_fill_then_drain_all();
    test_cancel_some_drain_some();
    test_not_cancellable();
    return 0;
}

#include <stdint.h>
#include <assert.h>
#include "cancellable.h"

#define COUNT 32
#define PARANOID true

int main(void)
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
    return 0;
}

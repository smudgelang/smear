#include <assert.h>
#include <stdint.h>
#include "cancellable.h"

#define COUNT 32
#define PARANOID false

int main(void)
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

    return 0;
}

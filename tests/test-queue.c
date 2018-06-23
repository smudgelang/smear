#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include "../src/queue/queue.h"

void test_growing(void)
{
    queue_t *q;

    uintptr_t i, n;
    size_t grower;
    bool st;
    size_t s;

    for (grower = 1; grower < 1025; grower++)
    {
        q = newq();
        assert(q != NULL);

        for (i = 0; i < grower; i++)
        {
            st = enqueue(q, (const void **)i);
            s = size(q);
            if (s != (size_t)i + 1)
            {
                printf("Size mismatch: expected %ld, got %zu.\n", i + 1, s);
                assert(false);
            }
            if (!st)
            {
                printf("Failed to enqueue %ld\n", i);
                assert(false);
            }
        }
        
        for (i = 0; i < grower; i++)
        {
            st = dequeue(q, (const void **)&n);
            if (!st)
            {
                printf("Failed to dequeue %ld\n", i);
                assert(false);
            }
            if (n != i)
            {
                printf("dequeued %ld, expected %ld.\n", n, i);
                assert(false);
            }
        }
        freeq(q);
    }
}

int main(void)
{
    queue_t *q;
    size_t s;
    intptr_t i, n;
    bool st;

    test_growing();
    q = newq();
    if (q == NULL)
    {
        printf("Failed to allocate a queue.\n");
        assert(false);
        return -10;
    }

    for (i = 0; i < 111; i++)
    {
        st = enqueue(q, (const void **)i);
        s = size(q);
        if (s != (size_t)i + 1)
        {
            printf("Size mismatch: expected %ld, got %zu.\n", i + 1, s);
            assert(false);
            return -3;
        }
        if (!st)
        {
            printf("Failed to enqueue %ld\n", i);
            assert(false);
            return -6;
        }
    }

    for (i = 0; i < 111; i++)
    {
        st = dequeue(q, (const void **)&n);
        if (!st)
        {
            printf("Failed to dequeue %ld\n", i);
            assert(false);
            return -2;
        }
        if (n != i)
        {
            printf("dequeued %ld, expected %ld.\n", n, i);
            assert(false);
            return -4;
        }
    }

    for (i = 0; i < 1000; i++)
    {
        s = size(q);
        if (s != (size_t)i)
        {
            printf("Size mismatch: expected %ld, got %zu.\n", i, s);
            assert(false);
            return -3;
        }
        st = enqueue(q, (void *)i);
        if (!st)
        {
            printf("Failed to enqueue %ld\n", i);
            assert(false);
            return -1;
        }
    }

    for (i = 0; i < 1000; i++)
    {
        st = dequeue(q, (const void **)&n);
        if (!st)
        {
            printf("Failed to dequeue %ld\n", i);
            assert(false);
            return -2;
        }
        if (n != i)
        {
            printf("dequeued %ld, expected %ld.\n", n, i);
            assert(false);
            return -4;
        }
    }

    s = size(q);
    if (s != 0)
    {
        printf("Size mismatch, expecting 0 got %zu", s);
        assert(false);
        return -5;
    }

    for (i = 0; i < 1000; i++)
    {
        s = size(q);
        if (s != (size_t)i)
        {
            printf("Size mismatch: expected %ld, got %zu.\n", i, s);
            assert(false);
            return -3;
        }
        st = enqueue(q, (const void *)i);
        if (!st)
        {
            printf("Failed to enqueue %ld\n", i);
            assert(false);
            return -1;
        }
    }

    for (i = 0; i < 1000; i++)
    {
        st = dequeue(q, (const void **)&n);
        if (!st)
        {
            printf("Failed to dequeue %ld\n", i);
            assert(false);
            return -2;
        }
        if (n != i)
        {
            printf("dequeued %ld, expected %ld.\n", n, i);
            assert(false);
            return -4;
        }
    }

    s = size(q);
    if (s != 0)
    {
        printf("Size mismatch, expecting 0 got %zu", s);
        assert(false);
        return -5;
    }

    freeq(q);
    q = NULL;
    return 0;
}

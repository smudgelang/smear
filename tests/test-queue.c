#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "queue.h"

int main(void)
{
    queue_t *q;
    size_t s;
    intptr_t i, n;
    bool st;

    q = newq();
    if (q == NULL)
    {
        printf("Failed to allocate a queue.\n");
        return -10;
    }
    for (i = 0; i < 1000; i++)
    {
        s = size(q);
        if (s != (size_t)i)
        {
            printf("Size mismatch: expected %ld, got %zu.\n", i, s);
            return -3;
        }
        st = enqueue(q, (void *)i);
        if (!st)
        {
            printf("Failed to enqueue %ld\n", i);
            return -1;
        }
    }

    for (i = 0; i < 1000; i++)
    {
        st = dequeue(q, (const void **)&n);
        if (!st)
        {
            printf("Failed to dequeue %ld\n", i);
            return -2;
        }
        if (n != i)
        {
            printf("dequeued %ld, expected %ld.\n", n, i);
            return -4;
        }
    }

    s = size(q);
    if (s != 0)
    {
        printf("Size mismatch, expecting 0 got %zu", s);
        return -5;
    }

    for (i = 0; i < 1000; i++)
    {
        s = size(q);
        if (s != (size_t)i)
        {
            printf("Size mismatch: expected %ld, got %zu.\n", i, s);
            return -3;
        }
        st = enqueue(q, (const void *)i);
        if (!st)
        {
            printf("Failed to enqueue %ld\n", i);
            return -1;
        }
    }

    for (i = 0; i < 1000; i++)
    {
        st = dequeue(q, (const void **)&n);
        if (!st)
        {
            printf("Failed to dequeue %ld\n", i);
            return -2;
        }
        if (n != i)
        {
            printf("dequeued %ld, expected %ld.\n", n, i);
            return -4;
        }
    }

    s = size(q);
    if (s != 0)
    {
        printf("Size mismatch, expecting 0 got %zu", s);
        return -5;
    }


    freeq(q);
    q = NULL;
    return 0;
}

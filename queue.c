/* Copyright 2017 Bose Corporation.
 * This software is released under the 3-Clause BSD License.
 * The license can be viewed at https://github.com/Bose/Smudge/blob/master/LICENSE
 */

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include "queue.h"

struct queue_s
{
    queue_t *next;
    void *val;
    bool empty;
    pthread_mutex_t mutex;
};

static queue_t *tail(queue_t *head)
{
    queue_t *cursor;
    for (cursor = head; cursor->next != NULL; cursor = cursor->next)
        ;
    return cursor;
}

#if 0 /* Useful for stuf we don't do right now. */
static bool in(queue_t *head, void *val)
{
    queue_t *cursor;

    if (head->empty)
        return false;
    for (cursor = head; cursor != NULL; cursor = cursor->next)
    {
        if (cursor->val == val)
            return true;
    }
    return false;
}
#endif

static queue_t *new_node(void)
{
    queue_t *node;

    node = malloc(sizeof(queue_t));
    if (node == NULL)
        return NULL;
    memset(node, 0, sizeof(*node));
    node->empty = true;
    return node;
}

queue_t *newq(void)
{
    queue_t *node;

    node = new_node();
    pthread_mutex_init(&node->mutex, NULL);
    return node;
}

void freeq(queue_t *head)
{
    queue_t *freeNext, *freeMe;

    assert(pthread_mutex_lock(&head->mutex) == 0);
    pthread_mutex_unlock(&head->mutex);
    pthread_mutex_destroy(&head->mutex);
    freeMe = head;
    while (freeMe != NULL)
    {
        freeNext = head->next;
        free(freeMe);
        freeMe = freeNext;
    }
    
    return;
}

bool enqueue(queue_t *head, void *value)
{
    queue_t *next;
    queue_t *parent;
    bool success;

    success = false;
    if (pthread_mutex_lock(&head->mutex) != 0)
        goto done;
    next = NULL;
    parent = tail(head);
    if (parent->empty == false)
    {
        next = new_node();
        if (next == NULL)
            goto error;
        parent->next = next;
    }
    else
    {
        next = parent;
    }
    next->empty = false;
    next->val = value;
    success = true;
error:
    pthread_mutex_unlock(&head->mutex);
done:
    return success;
}

bool dequeue(queue_t *head, void **value)
{
    queue_t *oldNext;
    bool success;

    success = false;
    if (pthread_mutex_lock(&head->mutex) != 0)
        goto error_unlocked;
    if (head->empty)
        goto error;
    *value = head->val;
    if (head->next == NULL)
    {
        head->empty = true;
    }
    else
    {
        oldNext = head->next;
        memcpy(head, oldNext, sizeof(queue_t));
        free(oldNext);
    }
    success = true;
error:
    pthread_mutex_unlock(&head->mutex);
error_unlocked:
    return success;
}

size_t size(const queue_t *head)
{
    size_t count;
    const queue_t *cursor;

    count = 0;
    if (pthread_mutex_lock((pthread_mutex_t *)&head->mutex) != 0)
    {
        assert(false);
        goto error_unlocked;
    }
    for (cursor = head; cursor != NULL; cursor = cursor->next)
    {
        if (!cursor->empty)
            count++;
    }
    pthread_mutex_unlock((pthread_mutex_t *)&head->mutex);
error_unlocked:
    return count;
}

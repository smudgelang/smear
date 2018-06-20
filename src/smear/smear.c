#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include "queue.h"

#define ERROR_MSG(msg) fprintf(stderr, "%s - %s\n", __FUNCTION__, msg)

typedef void (*handler_t)(const void *);

typedef struct
{
    const void *wrapper;
    handler_t handler;
} mq_msg_t;

static queue_t *q;
static pthread_t tid;

/* The plan:
 *
 * Put messages received onto a queue inside the _Send_Message
 * function; pop messages off the queue and send them on using the
 * _Handle_Message function. Messages sent come with pointers to
 * wrappers (cast to void) and pointers to the appropriate
 * _Handle_Message function. This can all just use the queue
 * implementation from queue.c.
 */

static void flushEventQueue(void)
{
    bool success;
    mq_msg_t *qmsg;

    while(size(q) > 0)
    {
        success = dequeue(q, (const void **)&qmsg);
        if (!success)
        {
            ERROR_MSG("Failed to dequeue element.");
            exit(-1);
        }
        qmsg->handler(qmsg->wrapper);
        free(qmsg);
    }
}

static void *mainloop(void *unused)
{
    while (true)
    {
        flushEventQueue();
        sleep(0);
    }
    return NULL;
}

void SMUDGE_debug_print(const char *fmt, const char *a1, const char *a2)
{
    fprintf(stderr, fmt, a1, a2);
}

void SMUDGE_free(const void *a1)
{
    free((void *)a1);
}

void SMUDGE_panic(void)
{
    exit(-1);
}

void SMUDGE_panic_print(const char *fmt, const char *a1, const char *a2)
{
    SMUDGE_debug_print(fmt, a1, a2);
    SMUDGE_panic();
}

void SRT_send_message(const void *msg, void (handler)(const void *))
{
    mq_msg_t *qmsg;

    if (msg == NULL)
    {
        ERROR_MSG("Null message sent.");
        exit(-3);
    }
    qmsg = malloc(sizeof(mq_msg_t));
    if (qmsg == NULL)
    {
        ERROR_MSG("Failed to allocate wrapper memory.");
        exit(-2);
    }
    qmsg->wrapper = msg;
    qmsg->handler = handler;
    if (!enqueue(q, qmsg))
    {
        ERROR_MSG("Failed to enqueue message.");
        exit(-2);
    }
}

void SRT_init(void)
{
    q = newq();
}

void SRT_run(void)
{
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_create(&tid, &attr, mainloop, NULL);
    pthread_attr_destroy(&attr);
}

static void SRT_join(void)
{
    void *rv;
    pthread_join(tid, &rv);
}

void SRT_wait_for_idle(void)
{
    return wait_empty(q);
}

void SRT_stop(void)
{
    void *rv;
    pthread_cancel(tid);
    pthread_join(tid, &rv);
    freeq(q);
}

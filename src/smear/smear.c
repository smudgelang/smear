#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include "smeartime.h"
#include "cancellable.h"
#include "smear/smear.h"

#define EXPORT_SYMBOL __attribute__((visibility ("default")))
#ifdef __STRICT_ANSI__
#define ERROR_MSG(msg) fprintf(stderr, "%s:%d - %s\n", __FILE__, __LINE__, msg)
#else
#define ERROR_MSG(msg) fprintf(stderr, "%s - %s\n", __FUNCTION__, msg)
#endif

EXPORT_SYMBOL const char *SRT_get_version(void)
{
    return SMEAR_VERSION;
}

#define NS_PER_MS 1000000
#define NANOSECONDS_PER_SECOND 1000000000

typedef void (*handler_t)(const void *);

typedef struct
{
    const void *wrapper;
    handler_t handler;
} mq_msg_t;

static event_queue_t *q;
static pthread_t tid;
static sem_t idle_sem;
static sem_t done;
static pthread_cond_t sleep_cv;
static pthread_mutex_t sleep_mx;

static void wait_wake(void)
{
    abs_time_t in_one_ms = time_add(get_now_real_ns(), NS_PER_MS);
    struct timespec soon = {
        .tv_sec  = in_one_ms / NANOSECONDS_PER_SECOND,
        .tv_nsec = in_one_ms % NANOSECONDS_PER_SECOND
    };

    pthread_mutex_lock(&sleep_mx);
    pthread_cond_timedwait(&sleep_cv, &sleep_mx, &soon);
    pthread_mutex_unlock(&sleep_mx);
}

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
    mq_msg_t *qmsg;

    while (true)
    {
        qmsg = eq_next_event(q, get_now_ns());
        if (qmsg == NULL)
            break;
        qmsg->handler(qmsg->wrapper);
        free(qmsg);
    }
}

static void *mainloop(void *unused)
{
    while (true)
    {
        flushEventQueue();
        sem_post(&idle_sem);
        if (sem_trywait(&done) == 0)
            return NULL;
        wait_wake();
        sem_wait(&idle_sem);
    }
    return NULL;
}

static void SRT_join(void)
{
    void *rv;
    pthread_join(tid, &rv);
}

static mq_msg_t *getQMsg(const void *msg, void (handler)(const void *))
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

    return qmsg;
}

EXPORT_SYMBOL void SMUDGE_debug_print(const char *fmt, const char *a1,
                                      const char *a2)
{
    fprintf(stderr, fmt, a1, a2);
}

EXPORT_SYMBOL void SMUDGE_free(const void *a1)
{
    free((void *)a1);
}

EXPORT_SYMBOL void SMUDGE_panic(void)
{
    exit(-1);
}

EXPORT_SYMBOL void SMUDGE_panic_print(const char *fmt, const char *a1,
                                      const char *a2)
{
    SMUDGE_debug_print(fmt, a1, a2);
    SMUDGE_panic();
}

EXPORT_SYMBOL void SRT_send_message(const void *msg,
                                    void (handler)(const void *))
{
    mq_msg_t *qmsg;

    qmsg = getQMsg(msg, handler);
    if (!eq_post(q, qmsg, get_now_ns()))
    {
        ERROR_MSG("Failed to enqueue message.");
        exit(-2);
    }
#if 0 // Helgrind claims that I'm doing something wrong here, and
      // signalling isn't strictly necessary for this stuff to work;
      // it's just a performance optimization. So until we figure out
      // what's wrong, I'm leaving this code disabled.
    pthread_mutex_lock(&sleep_mx);
    pthread_cond_signal(&sleep_cv);
    pthread_mutex_unlock(&sleep_mx);
#endif
}

EXPORT_SYMBOL cancel_token_t SRT_send_later(const void *msg,
                                            void (handler)(const void *),
                                            time_delta_t delay_ms)
{
    mq_msg_t *qmsg;
    cancellable_id_t id;

    qmsg = getQMsg(msg, handler);
    id = eq_schedule(q, qmsg, get_now_ns() + delay_ms * NS_PER_MS);
    if (id == NOT_CANCELLABLE || id == SCHEDULE_FAIL)
    {
        ERROR_MSG("Failed to schedule message.");
        exit(-2);
    }
    return id;
}

EXPORT_SYMBOL void SRT_cancel(cancel_token_t id)
{
    mq_msg_t *qmsg;

    if (eq_cancel_or_release(q, id, (void **)&qmsg) != SUCCESS)
    {
        ERROR_MSG("Failed to release event.");
        exit(-4);
    }
    if (qmsg != NULL)
        SMUDGE_free(qmsg);
}

EXPORT_SYMBOL void SRT_init(void)
{
    q = eq_new();
    sem_init(&idle_sem, 0, 0);
    sem_init(&done, 0, 0);
    pthread_mutex_init(&sleep_mx, NULL);
    pthread_cond_init(&sleep_cv, NULL);
}

EXPORT_SYMBOL void SRT_run(void)
{
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_create(&tid, &attr, mainloop, NULL);
    pthread_attr_destroy(&attr);
}

EXPORT_SYMBOL void SRT_wait_for_idle(void)
{
    sem_wait(&idle_sem);
    sem_post(&idle_sem);
    return;
}

EXPORT_SYMBOL void SRT_wait_for_empty(void)
{
    bool loop;

    loop = true;
    while (loop)
    {
        eq_wait_empty(q);
        sem_wait(&idle_sem);
        if (eq_empty(q))
        {
            loop = false;
        }
        sem_post(&idle_sem);
    }
}

EXPORT_SYMBOL void SRT_stop(void)
{
    void *rv;

    sem_post(&done);
    pthread_join(tid, &rv);
    eq_free(q);
    sem_destroy(&idle_sem);
    sem_destroy(&done);
    pthread_mutex_destroy(&sleep_mx);
    pthread_cond_destroy(&sleep_cv);
}

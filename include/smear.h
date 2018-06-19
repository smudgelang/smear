#ifndef __SMEAR_H__
#define __SMEAR_H__

#include <string.h>
#include <stdlib.h>

#define CAT(a, b) a ## b

/* For each state machine, instantiate this macro once with the
 * mangled name of the state machine as its argument. */
#define SRT_HANDLERS(m)                                                \
    static void CAT(m, _handler) (const void *msg)                     \
    {                                                                  \
        CAT(m, _Handle_Message) (*( CAT(m, _Event_Wrapper) *)msg);     \
        CAT(m, _Free_Message) (*( CAT(m, _Event_Wrapper) *)msg);       \
        free((void *)msg);                                             \
    }                                                                  \
    void CAT(m, _Send_Message) ( CAT(m, _Event_Wrapper e) )            \
    {                                                                  \
        void *msg;                                                     \
        msg = malloc(sizeof(e));                                       \
        memcpy(msg, &e, sizeof(e));                                    \
        SRT_send_message(msg, CAT(m, _handler));                       \
    }

/* Call SRT_init before sending any events to any state machines. */
void SRT_init(void);

/* SRT_run starts a thread that will run state machines. Events will
 * be processed after this function is called, whether they were sent
 * before or after. Events are guaranteed to be processed in the order
 * they're received. */
void SRT_run(void);

/* SRT_stop ends execution of the runtime and frees associated
 * resources. */
void SRT_stop(void);

/* SRT_continue executes until there are no pending events. */
void SRT_wait_for_idle(void);

/* Don't call this directly. */
void SRT_send_message(const void *msg, void (handler)(const void *));

#endif

#ifndef __SMEAR_H__
#define __SMEAR_H__

#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#define CAT(a, b) a ## b
#define CAT3(a, b, c) a ## b ## c
#define CAT4(a, b, c, d) a ## b ## c ## d

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
        if (msg == NULL) SMUDGE_panic();                               \
        memcpy(msg, &e, sizeof(e));                                    \
        SRT_send_message(msg, CAT(m, _handler));                       \
    }

// cancel_token_t SRT_delayed_send(<machine name>, <event name>, void *msg,
//                                 uint64_t delay_msg);
#define SRT_delayed_send(machine, event_name, msg, delay_ms)            \
    ({                                                                  \
        CAT(machine, _Event_Wrapper) *wrapper;                          \
        wrapper = malloc(sizeof(CAT(machine, _Event_Wrapper)));         \
        if (wrapper == NULL) SMUDGE_panic();                            \
        wrapper->id = CAT4(EVID_, machine, _, event_name);              \
        CAT(wrapper->event.e_, event_name) = msg;                       \
        SRT_send_later(wrapper,                                         \
                       (void (*)(const void *))CAT3(machine, _, event_name), \
                        delay_ms);                                      \
    })

typedef size_t cancel_token_t;

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

/* Block until there are no pending events, then return. */
void SRT_wait_for_idle(void);

/* Don't call this directly. */
void SRT_send_message(const void *msg, void (handler)(const void *));

cancel_token_t SRT_send_later(const void *msg, void (handler)(const void *),
                              uint64_t delay_ms);

void SRT_cancel(cancel_token_t id);
#endif

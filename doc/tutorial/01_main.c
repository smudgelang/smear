/* 01_main.c */
#include <stdio.h>
#include <smear.h>
#include "01_pinball.h"
#include "01_pinball_ext.h"

SRT_HANDLERS(pinball)

void print_state(void)
{
    printf("Current state: %s\n", pinball_Current_state_name());
}

int main(void)
{
    SRT_init();

    printf("Sending coin event.\n");
    pinball_coin(NULL);

    printf("Starting runtime.\n");
    SRT_run();
    print_state();

    printf("Waiting for events to be handled.\n");
    SRT_wait_for_idle();
    print_state();

    printf("Sending drain event.\n");
    pinball_drain(NULL);
    print_state();

    printf("Waiting for events to be handled.\n");
    SRT_wait_for_idle();
    print_state();

    SRT_wait_for_idle();

    SRT_stop();
    return 0;
}

/* 00_main.c */
#include <smear.h>
#include "00_pinball.h"
#include "00_pinball_ext.h"

SRT_HANDLERS(pinball)

int main(void)
{
    SRT_init();
    SRT_run();
    SRT_wait_for_idle();
    SRT_stop();
    return 0;
}

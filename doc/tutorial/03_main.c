/* 03_main.c */
#include <stdio.h>
#include <smear.h>
#include "03_pinball.h"
#include "03_pinball_ext.h"

SRT_HANDLERS(pinball)

void incScore(const pinball_target_t *unused)
{
    static int score = 0;

    score++;
    printf("ding ding, new score: %d\n", score);
}

int main(void)
{
    SRT_init();
    SRT_run();
    pinball_coin(NULL);
    pinball_target(NULL);
    pinball_target(NULL);
    pinball_drain(NULL);
    SRT_wait_for_idle();
    SRT_stop();
    return 0;
}

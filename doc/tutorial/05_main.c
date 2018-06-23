/* 05_main.c */
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <smear.h>
#include "05_pinball.h"
#include "05_pinball_ext.h"

SRT_HANDLERS(pinball)

static int score;
static int highScore;
static timer_t tiltTimer;

static void sendTimerExpired(union sigval unused)
{
    pinball_timerExpired(NULL);
}

void startTimer(void)
{
    struct sigevent sev;

    sev.sigev_notify = SIGEV_THREAD;
    sev.sigev_notify_function = sendTimerExpired;
    sev.sigev_notify_attributes = NULL;
    timer_create(CLOCK_MONOTONIC, &sev, &tiltTimer);
}

void displayError(void)
{
    printf("TILT!\n");
}

void lockPaddles(void)
{
    printf("Locking paddles.\n");
}

void sadSound(const pinball_drain_t *unused)
{
    printf("\nAwwwwwww\n");
}

void unlockPaddles(void)
{
    printf("Unlocking paddles. Whirrrrrr\n");
}

void incScore(const pinball_target_t *unused)
{
    score++;
    printf("Ding");
}

void displayScore(void)
{
    printf("Score: %d\n", score);
    if (score > highScore)
    {
        highScore = score;
        printf("A new high score!\n");
    }
    else
        printf("High score: %d\n", highScore);
}

void releaseBall(void)
{
    printf("Dropping ball.\n");
}
void resetScore(void)
{
    score = 0;
}
void startSound(void)
{
    printf("Ding ding ding ding ding\n");
}

int main(void)
{
    SRT_init();
    SRT_run();
    pinball_coin(NULL);
    pinball_plunger(NULL);
    pinball_target(NULL);
    pinball_target(NULL);
    pinball_drain(NULL);

    pinball_coin(NULL);
    pinball_plunger(NULL);
    for (int i=0; i < 100; i++)
        pinball_target(NULL);
    pinball_drain(NULL);

    pinball_tilt(NULL);
    pinball_coin(NULL);
    pinball_plunger(NULL);
    pinball_target(NULL);
    pinball_drain(NULL);

    SRT_wait_for_idle();
    SRT_stop();
    return 0;
}

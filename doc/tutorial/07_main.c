/* 07_main.c */
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <smear.h>
#include "07_pinball.h"
#include "07_pinball_ext.h"

SRT_HANDLERS(pinball)

static int score;
static int highScore;
static timer_t tiltTimer;

static void sendTimerExpired(union sigval unused)
{
    printf("Timer expiring.\n");
    pinball_timerExpired(NULL);
}

static void setupTimer(void)
{
    struct sigevent sev;
    sev.sigev_notify = SIGEV_THREAD;
    sev.sigev_notify_function = sendTimerExpired;
    sev.sigev_notify_attributes = NULL;
    timer_create(CLOCK_MONOTONIC, &sev, &tiltTimer);
}

static void teardownTimer(void)
{
    if (timer_delete(tiltTimer) != 0)
        perror("Failed to delete timer.");
}

void startTimer(void)
{
    struct itimerspec delay;

    delay.it_value.tv_sec = 2;
    delay.it_value.tv_nsec = 0;
    delay.it_interval.tv_sec = 0;
    delay.it_interval.tv_nsec = 0;
    timer_settime(tiltTimer, 0, &delay, NULL);
}

void rejectCoin(const pinball_coin_t *unused)
{
    printf("Coin at a bad time. Dropping it.\n");
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

void displayScore(const pinball_drain_t *unused)
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
    setupTimer();
    SRT_init();
    SRT_run();

    pinball_coin(NULL);
    pinball_plunger(NULL);
    pinball_coin(NULL);
    for (int i = 0; i < 4; i++)
        pinball_target(NULL);
    pinball_drain(NULL);

    pinball_coin(NULL);
    pinball_plunger(NULL);
    pinball_target(NULL);
    pinball_target(NULL);
    pinball_tilt(NULL);
    pinball_coin(NULL);
    pinball_plunger(NULL);
    pinball_target(NULL);
    pinball_drain(NULL);

    SRT_wait_for_idle();
    sleep(3);
    
    SRT_stop();
    teardownTimer();
    return 0;
}

/* 09_main.c */
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <smear.h>
#include "09_pinball.h"
#include "09_pinball_ext.h"

SRT_HANDLERS(pinball)
SRT_HANDLERS(flippers)

struct pinball_target_t
{
    int value;
};

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

static pinball_target_t *newTarget(int val)
{
    pinball_target_t *tgt;
    tgt = malloc(sizeof(*tgt));
    if (tgt != NULL)
        tgt->value = val;
    return tgt;
}

void rejectCoin(const pinball_coin_t *unused)
{
    printf("Coin at a bad time. Dropping it.\n");
}

void displayError(void)
{
    printf("TILT!\n");
}

void flipLeft(const flippers_left_t *unused)
{
    printf("fLip");
}

void flipRight(const flippers_right_t *unused)
{
    printf("flipR");
}

void clickLock(void)
{
    printf("Locking flippers.\n");
}

void soundFree(void)
{
    printf("Flippers away!\n");
}

void sadSound(const pinball_drain_t *unused)
{
    printf("\nAwwwwwww\n");
}

void unlockPaddles(void)
{
    printf("Unlocking paddles. Whirrrrrr\n");
}

void incScore(const pinball_target_t *tgt)
{
    if (tgt != NULL)
        score += tgt->value;
    else
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
    SRT_wait_for_idle();
    for (int i = 0; i < 4; i++)
    {
        flippers_left(NULL);
        pinball_target(newTarget((i + 2) * 8));
    }
    flippers_right(NULL);
    pinball_drain(NULL);

    pinball_coin(NULL);
    pinball_plunger(NULL);
    pinball_target(newTarget(100));
    pinball_target(newTarget(100));
    pinball_tilt(NULL);
    pinball_coin(NULL);
    pinball_plunger(NULL);
    pinball_target(newTarget(1000));
    pinball_drain(NULL);

    SRT_wait_for_idle();
    sleep(3);
    SRT_wait_for_idle();
    
    SRT_stop();
    teardownTimer();
    return 0;
}

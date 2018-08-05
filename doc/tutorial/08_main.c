/* 08_main.c */
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <smear.h>
#include "08_pinball.h"
#include "08_pinball_ext.h"

SRT_HANDLERS(pinball)

static int score;
static int highScore;
static cancel_token_t timer;

void startTimer(void)
{
    timer = SRT_delayed_send(pinball, timerExpired, NULL, 2000);
}

void cancelTimer(void)
{
    SRT_cancel(timer);
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
    return 0;
}

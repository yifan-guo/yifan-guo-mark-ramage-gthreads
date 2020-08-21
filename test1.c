//
// Created by Yifan Guo on 11/20/16.
//
#include <unistd.h>
#include "uthash.h"
#include "gthr.c"


int main(void)
{
    //Setting new timerval and itimerval
    struct timeval period;
    struct itimerval repeatingTimer;
    period.tv_sec = TICK_PERIOD_S;
    period.tv_usec = TICK_PERIOD_U;

    repeatingTimer.it_interval = period;
    repeatingTimer.it_value = period;

    gtinit(0, RR);       //set gtcur = gttbl[0] and gttbl[0] to RUNNING
    init();         //allocate the ready queues

    //set up the signal to occur every TICK_PERIOD_S seconds
    signal(SIGALRM, sigFunc);
    setitimer(ITIMER_REAL, &repeatingTimer, NULL);

    printf("Testing two high priority IO round robin threads, with 1 low priority FIFO thread\n");
    gtgo(f, 50, FIFO);          //initialize gthread and place it on its ready queuee
    gtgo(f_io, 60, RR);        //initialize gthread and place it on its ready queuee
    gtgo(f_io, 60, RR);        //initialize gthread and place it on its ready queuee
    gtret(1);       //gttbl[0] calls yield

}

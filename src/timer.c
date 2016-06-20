#define _GNU_SOURCE /* Required to use signals */

#include "timer.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>

void init_timer(struct timer_t *timer) { memset(timer, 0, sizeof(*timer)); }

struct timer_t default_timer = {.do_signal = 0, .max_signal = -1};

static void alarm_handler(int sig) { default_timer.do_signal = 1; }

int set_timer_with_handler(struct timer_t *timer, unsigned int ms,
                            handler_t handler)
{
    struct itimerval itv;
    struct sigaction sa;

    sigemptyset(&sa.sa_mask);

    sa.sa_flags = SA_RESTART;
    sa.sa_handler = handler;

    if (sigaction(SIGALRM, &sa, NULL) == -1) {
        return -1; // unable to set handler
    }

    itv.it_value.tv_sec = 0;
    itv.it_value.tv_usec = 1000 * ms;
    itv.it_interval.tv_sec = 0;
    itv.it_interval.tv_usec = 1000 * ms;

    if (setitimer(ITIMER_REAL, &itv, 0) == -1) {
        return -2; // unable to set timer
    }

    return 0;
}

int set_timer(unsigned int ms)
{
    default_timer.handler = alarm_handler;
    return set_timer_with_handler(&default_timer, ms, default_timer.handler);
}

void timer_reset_signal(struct timer_t *timer)
{
    timer->do_signal = 0;

    if (timer->max_signal > 0 && --timer->max_signal == 0) {
        stop_timer();
    }
}

void reset_signal() { timer_reset_signal(&default_timer); }

void stop_timer()
{
    struct itimerval old;
    struct itimerval new;

    if (getitimer(ITIMER_REAL, &old) == -1) {
        printf("unable to stop timer, %s\n", strerror(errno));
        return;
    }

    memset(&new, 0, sizeof(new));

    if (setitimer(ITIMER_REAL, &new, NULL) == -1) {
        printf("unable to stop timer: %s\n", strerror(errno));
    }
}

int timer_got_signal(struct timer_t *timer) { return timer->do_signal; }

int got_signal() { return timer_got_signal(&default_timer); }

void set_default_signal_count(int count) { default_timer.max_signal = count; }

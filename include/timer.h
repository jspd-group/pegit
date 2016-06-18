#ifndef TIMER_H_
#define TIMER_H_

#include <sys/time.h>
#include <sys/types.h>
#include <ctype.h>
#include <signal.h>
#include <sys/signal.h>
#include <time.h>

typedef void (*handler_t)(int sig);

struct timer_t {
    struct itimerval val;
    int running;
    handler_t handler;
    int do_signal;
    int max_signal;
};

extern void stop_timer();

extern void init_timer(struct timer_t *timer);

extern int set_timer__with_handler(struct timer_t *timer, unsigned int ms,
                    handler_t handler);

extern int set_timer(unsigned int ms);

extern void timer_reset_signal(struct timer_t *timer);
extern void reset_signal();
extern int timer_got_signal(struct timer_t *timer);

extern int got_signal();

extern void set_default_signal_count(int count);

#endif

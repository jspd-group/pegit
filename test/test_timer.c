#include "timer.h"
#include <stdio.h>

int main()
{
    unsigned long long i = 0;
    int count = 10;
    set_timer(100);
    set_default_signal_count(100);
    while (1) {
        if (got_signal()) {
            printf("\r%llu, %d", i, count);
            fflush(stdout);
            reset_signal();
        }
        i++;
    }

    return 0;
}

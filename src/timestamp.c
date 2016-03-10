#include "timestamp.h"

void time_stamp_init(struct timestamp *ts)
{
    ts->_time = time(NULL);    /* get current time */
    ts->_tm = localtime(&ts->_time);
}

void time_stamp_make(struct timestamp *ts)
{
    ts->_time = time(NULL);
    ts->_tm = localtime(&ts->_time);
}

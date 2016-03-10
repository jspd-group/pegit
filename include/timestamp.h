#ifndef TIME_STAMP_H_
#define TIME_STAMP_H_

#include "util.h"
#include "strbuf.h"

#include <time.h>

struct timestamp {
    time_t _time;
    struct tm *_tm;
};

extern void time_stamp_init(struct timestamp *);

extern void time_stamp_make(struct timestamp *);

#endif

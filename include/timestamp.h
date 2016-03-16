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

static inline void time_stamp_read(struct timestamp *ts, FILE *f)
{
    fread(&ts->_time, sizeof(time_t), 1, f);
}

static inline void write_time_stamp(struct timestamp *ts, FILE *f)
{
    fwrite(&ts->_time, sizeof(time_t), 1, f);
}

#endif

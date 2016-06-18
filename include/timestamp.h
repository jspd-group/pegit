#ifndef TIME_STAMP_H_
#define TIME_STAMP_H_

#include "strbuf.h"
#include "util.h"

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
    ts->_tm = localtime(&ts->_time);
}

static inline void write_time_stamp(struct timestamp *ts, FILE *f)
{
    fwrite(&ts->_time, sizeof(time_t), 1, f);
}

extern void time_stamp_humanise(struct timestamp *ts, struct strbuf *buf);

#endif

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

#define TIME_FORMAT "%A of %B %d, at %I:%M:%S %p"

void time_stamp_humanise(struct timestamp *ts, struct strbuf *buf)
{
    size_t size = 256, old = buf->len;
    ts->_tm = localtime(&ts->_time);
    if (strbuf_avail(buf) == 0) {
        strbuf_grow(buf, size);
    }
    for (;; size *= 2) {
        if (strftime(buf->buf + old, strbuf_avail(buf), TIME_FORMAT, ts->_tm))
            break;
        strbuf_setlen(buf, old);
        strbuf_grow(buf, size);
    }
}

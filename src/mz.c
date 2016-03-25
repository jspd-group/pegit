#include "mz.h"
#include "util.h"
#include "file.h"
#include <zlib.h>

#define CHUNK_LENGTH 16384

void z_stream_init(z_stream *stream)
{
    stream->zalloc = Z_NULL;
    stream->zfree = Z_NULL;
    stream->opaque = Z_NULL;
}

/* report a zlib or i/o error */
void zerr(int ret)
{
    fputs("zpipe: ", stderr);
    switch (ret) {
    case Z_STREAM_ERROR:
        die("invalid compression level\n");
        break;
    case Z_DATA_ERROR:
        die("invalid or incomplete deflate data\n");
        break;
    case Z_MEM_ERROR:
        die("out of memory\n");
        break;
    case Z_VERSION_ERROR:
        die("zlib version mismatch!\n");
    }
}


int __compress__(struct strbuf *src, struct strbuf *dest, int level)
{
    z_stream stream;
    z_stream_init(&stream);
    int ret, flush;
    unsigned have;
    ret = deflateInit(&stream, Z_DEFAULT_COMPRESSION);
    if (ret != Z_OK)
        zerr(ret);
    char buffer[CHUNK_LENGTH];
    stream.avail_in = src->len;
    stream.next_in = (Bytef *)src->buf;
    do {
        flush = (stream.avail_in < CHUNK_LENGTH) ? Z_FINISH : Z_NO_FLUSH;
        do {
            stream.avail_out = CHUNK_LENGTH;
            stream.next_out = (Bytef *)buffer;
            ret = deflate(&stream, flush);
            have = CHUNK_LENGTH - stream.avail_out;
            strbuf_add(dest, buffer, have);
        } while (stream.avail_out == 0);

        assert(stream.avail_in == 0);

    } while (flush != Z_FINISH);
    assert(ret == Z_STREAM_END);
    deflateEnd(&stream);
    return ret;
}

int compress_default(struct strbuf *src, struct strbuf *dest)
{
    return __compress__(src, dest, 9);
}

void uncompress_setup(z_stream *stream, struct strbuf *src, struct strbuf *dest)
{
    stream->zalloc = Z_NULL;
    stream->zfree = Z_NULL;
    stream->opaque = Z_NULL;
    stream->avail_in = src->len;
    stream->next_in = (Bytef *)src->buf;
}

int uncompress_chunk(z_stream *stream, struct strbuf *dest)
{
    char buffer[CHUNK_LENGTH] = {'\0'};
    stream->avail_out = CHUNK_LENGTH;
    stream->next_out = (Bytef *)buffer;
    int ret = inflate(stream, Z_NO_FLUSH);
    strbuf_add(dest, buffer, stream->next_out - (unsigned char *)buffer);
    return ret;
}

int decompress(struct strbuf *src, struct strbuf *dest)
{
    z_stream stream;
    int ret, count = 0;
    struct strbuf temp;
    strbuf_init(&temp, 0);
    uncompress_setup(&stream, src, dest);
    ret = inflateInit(&stream);
    if (ret != Z_OK)
        zerr(ret);
    do {
        ret = uncompress_chunk(&stream, dest);
        if (ret == Z_BUF_ERROR) {
            (void)inflateEnd(&stream);
            return ret;
        }
    } while (ret != Z_STREAM_END);
    (void)inflateEnd(&stream);
    return ret;
}

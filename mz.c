#include "mz.h"
#include "util.h"

#define CHUNK_LENGTH 1024

void z_stream_init(z_stream *stream) {
  stream->zalloc = Z_NULL;
  stream->zfree = Z_NULL;
  stream->opaque = Z_NULL;
}

int compress(struct strbuf *src, struct strbuf *dest, int level) {
    z_stream stream;
    z_stream_init(&stream);
    int ret, flush, i;
    unsigned have;
    deflateInit(&stream, Z_DEFAULT_COMPRESSION);
    char buffer[CHUNK_LENGTH];

    do {
        stream.avail_in = src->len;
        stream.next_in = src->buf;
        flush = (stream.avail_in < CHUNK_LENGTH) ? Z_FINISH : Z_NO_FLUSH;

        do {
            stream.avail_out = CHUNK_LENGTH;
            stream.next_out = buffer;
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

int compress_default(struct strbuf *src, struct strbuf *dest) {
  return compress(src, dest, Z_DEFAULT_COMPRESSION);
}

void uncompress_setup(z_stream *stream, struct strbuf *src,
                      struct strbuf *dest){
  stream->zalloc = Z_NULL;
  stream->zfree = Z_NULL;
  stream->opaque = Z_NULL;
  stream->avail_in = src->len;
  stream->next_in = src->buf;
}

int uncompress_chunk(z_stream *stream, struct strbuf *dest) {
    char buffer[CHUNK_LENGTH] = { '\0' };
    stream->avail_out = CHUNK_LENGTH;
    stream->next_out = buffer;
    int ret = inflate(stream, Z_NO_FLUSH);
    strbuf_add(dest, buffer, stream->next_out - (unsigned char*)buffer);
    return ret;
}

int uncompress(struct strbuf *src, struct strbuf *dest) {
    z_stream stream;
    int ret, i = 1;
    struct strbuf temp;
    strbuf_init(&temp, 0);
    uncompress_setup(&stream, src, dest);
    inflateInit(&stream);
    do {
        ret = uncompress_chunk(&stream, dest);
    } while (ret != Z_STREAM_END);
    (void)inflateEnd(&stream);
    return ret;

}

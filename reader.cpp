
#include "reader.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

struct reader {
    void *ctx;
    size_t (*read)(void *ctx, char * ptr, size_t len);
    int (*close)(void *ctx);
};


size_t reader_read(struct reader * reader, char * ptr, size_t len) {
    return reader->read(reader->ctx, ptr, len);
}

int reader_close(struct reader * reader) {
    int ret = 0;
    if (reader->close != 0) {
        ret = reader->close(reader->ctx);
    }
    free(reader);
    return ret;
}

static size_t file_read(void *ctx, char * ptr, size_t len) {
    FILE * file = (FILE*)ctx;

    if (ptr != 0) {
        return fread(ptr, 1, len, file);
    } 

    long cur = ftell(file);
    fseek(file, len, SEEK_CUR);
    return ftell(file) - cur;
}

static int file_close(void *ctx) {
    int ret = 0;
    FILE * file = (FILE*)ctx;
    if (file != 0) {
        ret = fclose(file);
    }
    return ret;
}

struct reader * reader_from_file(const char * filename) {
    FILE * file = fopen(filename, "rb");
    if (file == 0) {
        return 0;
    }

    struct reader * reader = (struct reader *)malloc(sizeof(struct reader));
    memset(reader, 0, sizeof(struct reader));

    reader->ctx = file;
    reader->read = file_read;
    reader->close = file_close;

    return reader;
}

struct memory {
    const char * ptr;
    size_t len;
};

static size_t mem_read(void *ctx, char * ptr, size_t len) {
    struct memory * mem = (struct memory *)ctx;
    if (mem->len == 0) {
        return 0;
    }

    if (len > mem->len) {
        len = mem->len;
    }

    if (ptr != 0) {
        memcpy(ptr, mem->ptr, len);
    }

    mem->ptr += len;
    mem->len -= len;

    return len;
}

static int mem_close(void *ctx) {
    return 0;
}

struct reader * reader_from_memory(const char * ptr, size_t len) {
    struct reader * reader = (struct reader *)malloc(sizeof(struct reader) + sizeof(struct memory));
    memset(reader, 0, sizeof(struct reader));

    struct memory * mem = (struct memory*)(reader + 1);
    mem->ptr = ptr;
    mem->len = len;

    reader->ctx = mem;
    reader->read = mem_read;
    reader->close = mem_close;

    return reader;
}

void buffer_init(struct buffer * buffer, const char * ptr, size_t len) {
    buffer->ptr = ptr;
    buffer->len = len;
    buffer->pos = 0;
}

const char * buffer_read_bytes(struct buffer * buffer, size_t len)
{
    assert(len <= buffer->len - buffer->pos);
    const char * ptr = buffer->ptr + buffer->pos;
    buffer->pos += len;
    return ptr;
}

uint8_t buffer_read_u8(struct buffer * buffer)
{
    return *(uint8_t*)buffer_read_bytes(buffer, 1);
}

uint16_t buffer_read_u16(struct buffer * buffer)
{
    return *(uint16_t*)buffer_read_bytes(buffer, 2);
}

uint32_t buffer_read_u32(struct buffer * buffer)
{
    return *(uint32_t*)buffer_read_bytes(buffer, 4);
}

uint64_t buffer_read_u64(struct buffer * buffer)
{
    return *(uint64_t*)buffer_read_bytes(buffer, 8);
}

float buffer_read_f32(struct buffer * buffer)
{
    return *(float*)buffer_read_bytes(buffer, 4);
}

double buffer_read_f64(struct buffer * buffer)
{
    return *(double*)buffer_read_bytes(buffer, 8);
}

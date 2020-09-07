#ifndef _CLI_READER_H_
#define _CLI_READER_H_

#include <stdlib.h>
#include <stdint.h>

struct reader;

struct reader * reader_from_file(const char * filename);
struct reader * reader_from_memory(const char * ptr, size_t len);

size_t reader_read(struct reader * reader, char * ptr, size_t len);
int reader_close(struct reader * reader);


struct buffer {
    const char * ptr;
    size_t len;
    size_t pos;
};

void buffer_init(struct buffer * buffer, const char * ptr, size_t len);
const char * buffer_read_bytes(struct buffer * buffer, size_t len);
uint8_t buffer_read_u8(struct buffer * buffer);
uint16_t buffer_read_u16(struct buffer * buffer);
uint32_t buffer_read_u32(struct buffer * buffer);
uint64_t buffer_read_u64(struct buffer * buffer);
float buffer_read_f32(struct buffer * buffer);
double buffer_read_f64(struct buffer * buffer);


#endif

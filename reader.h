#ifndef _CLI_READER_H_
#define _CLI_READER_H_

#include <stdlib.h>

struct reader;

struct reader * reader_from_file(const char * filename);
struct reader * reader_from_memory(const char * ptr, size_t len);

size_t reader_read(struct reader * reader, char * ptr, size_t len);
int reader_close(struct reader * reader);


#endif

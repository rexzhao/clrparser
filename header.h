#ifndef _CLRPARSER_HEADER_H_
#define _CLRPARSER_HEADER_H_

#include <stdlib.h>

struct FieldInfo {
	char type;
    int size;
    const char * name;
};

struct Header {
    const struct FieldInfo * fields;

    const char * ptr;
    size_t len;
};

const char * header_init(struct Header * header, const struct FieldInfo * fields, const char * ptr);

size_t       header_get_size(struct Header * header);
size_t       header_get_field_size(struct Header * header, const char * field);
uint64_t     header_get_field_u64(struct Header * header, const char * field);
const char * header_get_field_str(struct Header * header, const char * field);

void header_dump(struct Header * header, const char * title);

#endif

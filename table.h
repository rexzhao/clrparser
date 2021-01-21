#ifndef _CLRPARSER_HEADER_H_
#define _CLRPARSER_HEADER_H_

#include <stdlib.h>

struct FieldInfo {
    char type;
    int size;
    const char * name;
	void * ctx;
};

struct Table {
    const struct FieldInfo * fields;
    const char * ptr;
    size_t cellSize;
    size_t rowCount;
};

const char * table_init(struct Table * table, const struct FieldInfo * fields, const char * ptr, int rowCount);

// size_t       table_get_field_size(struct Table * table, int row, const char * field);

uint64_t     table_get_field_u64(struct Table * table, int row, const char * field);
const char * table_get_field_str(struct Table * table, int row, const char * field);
void *       table_get_field_ctx(struct Table * table, int row, const char * field);

void table_dump(struct Table * table, const char * title);

#endif

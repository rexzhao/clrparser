#ifndef _CLRPARSER_CLR_H_
#define _CLRPARSER_CLR_H_

#include "pe.h"
#include "table.h"

struct Slice {
    const char * ptr;
    size_t size;
};

struct Context {
    struct PEFile * file;

    struct Slice stringHeap;
    struct Slice guidHeap;
    struct Slice blobHeap;
    struct Slice unicodeHeap;

    int HeapSizes;

    struct Table tables[64];

    struct FieldInfo fields[200]; // 160
    int filedUsed;
};

int read_clr(struct Context * context, struct PEFile * file);
void clr_dump_type(struct Context * context);
void clr_dump_method(struct Context * context, int methodIndex);

#endif

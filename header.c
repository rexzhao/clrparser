
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "header.h"

static size_t get_field_size(const struct FieldInfo * f) {
    return f->size;
}

size_t header_get_size(struct Header * header) {
    if (header == 0 || header->fields == 0) {
        return 0;
    }

    size_t s = 0;
    for(int i = 0; header->fields[i].type; i++) {
        s += get_field_size(header->fields + i);
    }

    return s;
}

size_t header_get_field_size(struct Header * header, const char * field) {
    for(int i = 0; header->fields[i].type; i++) {
        if (strcmp(header->fields[i].name, field) != 0) {
            continue;
        }

		return get_field_size(header->fields + i);
	}

	return 0;
}

const char * header_init(struct Header * header, const struct FieldInfo * fields, const char * ptr) {
    header->fields = fields; 
    header->len = header_get_size(header);
    header->ptr = ptr;
    return ptr + header->len;
}

uint64_t header_get_field_u64(struct Header * header, const char * field) {
    if (header->ptr == 0) {
        return 0;
    }

    const char * ptr = header->ptr;
    for(int i = 0; header->fields[i].type; i++) {
        if (strcmp(header->fields[i].name, field) != 0) {
            ptr += get_field_size(header->fields + i);
            continue;
        }

        switch(header->fields[i].size) {
            case 1: return *(uint8_t*)ptr;
            case 2: return *(uint16_t*)ptr;
            case 4: return *(uint32_t*)ptr;
            case 8: return *(uint64_t*)ptr;
            default: return 0;
        };
    }
    return 0;
}


const char * header_get_field_str(struct Header * header, const char * field) {
    if (header->ptr == 0) {
        return 0;
    }

    const char * ptr = header->ptr;
    for(int i = 0; header->fields[i].type; i++) {
        if (strcmp(header->fields[i].name, field) != 0) {
            ptr += get_field_size(header->fields + i);
            continue;
        }

        return ptr;
    }
    return 0;
}


void header_dump(struct Header * header, const char * title) {
    if (header != 0) {
        printf("-- %s --\n", title);
    }

    if (header->ptr == 0) {
        printf(" null \n");
        return;
    }

    const char * ptr = header->ptr;
    for(int i = 0; header->fields[i].type; i++) {
        switch(header->fields[i].type) {
            case 'u':
                switch(header->fields[i].size) {
                    case 1:  printf("%s %lu\n", header->fields[i].name, (uint64_t)(*(uint8_t *)ptr)); break;
                    case 2:  printf("%s %lu\n", header->fields[i].name, (uint64_t)(*(uint16_t*)ptr)); break;
                    case 4:  printf("%s %lu\n", header->fields[i].name, (uint64_t)(*(uint32_t*)ptr)); break;
                    case 8:  printf("%s %lu\n", header->fields[i].name, (uint64_t)(*(uint64_t*)ptr)); break;
                    default: assert(0);
                };
                break;
            case 'x':
                switch(header->fields[i].size) {
                    case 1:  printf("%s 0x%lx\n", header->fields[i].name, (uint64_t)(*(uint8_t *)ptr)); break;
                    case 2:  printf("%s 0x%lx\n", header->fields[i].name, (uint64_t)(*(uint16_t*)ptr)); break;
                    case 4:  printf("%s 0x%lx\n", header->fields[i].name, (uint64_t)(*(uint32_t*)ptr)); break;
                    case 8:  printf("%s 0x%lx\n", header->fields[i].name, (uint64_t)(*(uint64_t*)ptr)); break;
                    default: assert(0);
                };
                break;
            case 's':
                printf("%s %s\n", header->fields[i].name, ptr);
                break;
            default:
                assert(0);
        }

        ptr += get_field_size(header->fields + i);
    }

    printf("\n");

    return;
}

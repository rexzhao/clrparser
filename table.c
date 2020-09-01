
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "table.h"

static size_t get_field_size(const struct FieldInfo * f) {
    return f->size;
}

static size_t calc_cell_size(struct Table * table) {
    if (table == 0 || table->fields == 0) {
        return 0;
    }

    size_t s = 0;
    for(int i = 0; table->fields[i].type; i++) {
        s += get_field_size(table->fields + i);
    }

    return s;
}

static size_t table_get_field_size(struct Table * table, const char * field) {
    for(int i = 0; table->fields[i].type; i++) {
        if (strcmp(table->fields[i].name, field) != 0) {
            continue;
        }

		return get_field_size(table->fields + i);
	}

	return 0;
}

const char * table_init(struct Table * table, const struct FieldInfo * fields, const char * ptr, int rowCount) {
    table->fields = fields; 
    table->cellSize = calc_cell_size(table);

    table->ptr = ptr;
	table->rowCount = rowCount;

    return ptr + table->cellSize * rowCount;
}

uint64_t table_get_field_u64(struct Table * table, int row, const char * field) {
    if (table->ptr == 0) {
        return 0;
    }

    const char * ptr = table->ptr + table->cellSize * row;
    for(int i = 0; table->fields[i].type; i++) {
        if (strcmp(table->fields[i].name, field) != 0) {
            ptr += get_field_size(table->fields + i);
            continue;
        }

        switch(table->fields[i].size) {
            case 1: return *(uint8_t*)ptr;
            case 2: return *(uint16_t*)ptr;
            case 4: return *(uint32_t*)ptr;
            case 8: return *(uint64_t*)ptr;
            default: return 0;
        };
    }
    return 0;
}


const char * table_get_field_str(struct Table * table, int row, const char * field) {
    if (table->ptr == 0) {
        return 0;
    }

    const char * ptr = table->ptr  + table->cellSize * row;
    for(int i = 0; table->fields[i].type; i++) {
        if (strcmp(table->fields[i].name, field) != 0) {
            ptr += get_field_size(table->fields + i);
            continue;
        }

        return ptr;
    }
    return 0;
}


void table_dump(struct Table * table, const char * title) {
    if (table != 0) {
        printf("-- %s --\n", title);
    }

    if (table->ptr == 0) {
        printf(" null \n");
        return;
    }

	for (int j = 0; j < table->rowCount; j++) {

		const char * ptr = table->ptr + table->cellSize * j;

		printf("%d:\n", j);

		for(int i = 0; table->fields[i].type; i++) {
			switch(table->fields[i].type) {
				case 'u':
					switch(table->fields[i].size) {
						case 1:  printf("  %s %lu\n", table->fields[i].name, (uint64_t)(*(uint8_t *)ptr)); break;
						case 2:  printf("  %s %lu\n", table->fields[i].name, (uint64_t)(*(uint16_t*)ptr)); break;
						case 4:  printf("  %s %lu\n", table->fields[i].name, (uint64_t)(*(uint32_t*)ptr)); break;
						case 8:  printf("  %s %lu\n", table->fields[i].name, (uint64_t)(*(uint64_t*)ptr)); break;
						default: assert(0);
					};
					break;
				case 'x':
					switch(table->fields[i].size) {
						case 1:  printf("  %s 0x%lx\n", table->fields[i].name, (uint64_t)(*(uint8_t *)ptr)); break;
						case 2:  printf("  %s 0x%lx\n", table->fields[i].name, (uint64_t)(*(uint16_t*)ptr)); break;
						case 4:  printf("  %s 0x%lx\n", table->fields[i].name, (uint64_t)(*(uint32_t*)ptr)); break;
						case 8:  printf("  %s 0x%lx\n", table->fields[i].name, (uint64_t)(*(uint64_t*)ptr)); break;
						default: assert(0);
					};
					break;
				case 's':
					printf("  %s %s\n", table->fields[i].name, ptr);
					break;
				default:
					assert(0);
			}

			ptr += get_field_size(table->fields + i);
		}

	}

    printf("\n");

    return;
}

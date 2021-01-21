#include "reader.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>

#include "clr.h"
#include "pe.h"

#include "opcode.h"

static void work(const char * filename);

int main(int argc, const char * argv[])
{
    for(int i = 1; i < argc; i++) {
        work(argv[i]);
    }
    return 0;
}

static void work(const char * filename) 
{
	struct PEFile pe;
	if (read_pe_file(&pe, filename) != 0) {
		assert(0);
	}
	
	struct Context context;

	if (read_clr(&context, &pe) != 0) {
		assert(0);
	}

	clr_dump_type(&context);
}

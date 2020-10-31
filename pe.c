#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>

#include "header_pe.h"

static char * read_full_file(const char * filename, size_t * size) {
    FILE * file = fopen(filename, "rb");
    if (file == 0) {
        return 0;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return 0;
    }

    long fileSize = ftell(file);
    if (fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return 0;
    }

    char * fileMemory = (char*)malloc(fileSize);

    if (fread(fileMemory, 1, fileSize, file) != fileSize) {
        fclose(file);
        free(fileMemory);
        return 0;
    }

    fclose(file);

	if (size) *size = fileSize;

    return fileMemory;
}

int read_pe_file(struct PEFile * file, const char * filename)
{
    char ptr[4] = {0x80, 0x0, 0x0, 0x0};
    assert((*(uint32_t*)ptr) == 128);

	size_t fileSize;
    char * fileMemory = read_full_file(filename, &fileSize);
    if (fileMemory == 0) {
        return -1;
    }

    const char * ite = fileMemory;

    const char * MSDOSHeader = ite;
    ite += 128;

    size_t pe_offset = *(uint32_t*)(MSDOSHeader + 0X3c);
    ite = fileMemory + pe_offset;

    const char * PESignature = ite;
    ite += 4;

    if (memcmp(PESignature, "PE\0\0", 4) != 0) {
        printf(" read pe signature error %x, %x, %x, %x\n", PESignature[0], PESignature[1], PESignature[2], PESignature[3]);
		free(fileMemory);
        return -1; 
    }

	memset(file, 0, sizeof(struct PEFile));
	file->ptr = fileMemory;
	file->size = fileSize;

#define READ_HEADER(TYPE) file->TYPE = (struct T##TYPE*)ite;  ite = (const char *)(file->TYPE + 1);

	READ_HEADER(COFFFileHeader);

    uint64_t SizeOfOptionalHeader = file->COFFFileHeader->SizeOfOptionalHeader;

    uint64_t CLRRuntimeHeaderVirtualAddress = 0;
    uint64_t CLRRuntimeHeaderSize = 0;

    if (SizeOfOptionalHeader > 0) {
        const char * magic = ite;

		uint64_t NumberOfRvaAndSizes = 0;
        if (magic[0] == 0x0b && magic[1] == 0x01) { // 0x10b
			READ_HEADER(OptionalHeaderStandardFields_PE32);
			READ_HEADER(OptionalHeaderWindowsSpecificFields_PE32);

			NumberOfRvaAndSizes = file->OptionalHeaderWindowsSpecificFields_PE32->NumberOfRvaAndSizes;
        } else {
			READ_HEADER(OptionalHeaderStandardFields_PE32Plus);
			READ_HEADER(OptionalHeaderWindowsSpecificFields_PE32Plus);
			NumberOfRvaAndSizes = file->OptionalHeaderWindowsSpecificFields_PE32Plus->NumberOfRvaAndSizes;
        }

		file->IMAGE_DATA_DIRECTORY = (struct TIMAGE_DATA_DIRECTORY*)ite;
		ite += sizeof(struct TIMAGE_DATA_DIRECTORY) * NumberOfRvaAndSizes;
    }

	file->SectionTable = (struct TSectionTable*)ite;

	return 0;
}


const char * find_virtual_addr(struct PEFile * file, uint64_t VirtualAddress) {
	for (int i = 0; i < file->COFFFileHeader->NumberOfSections; i++) {
		struct TSectionTable * section = file->SectionTable + i;

		if ( section->VirtualAddress <= VirtualAddress && section->VirtualAddress + section->VirtualSize>= VirtualAddress) {
			const char * ptr = file->ptr + section->PointerToRawData + (VirtualAddress - section->VirtualAddress);
			return ptr;
		}
	}
	return 0;
}

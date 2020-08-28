

#include "reader.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "header_pe.h"
#include "header_clr.h"


static void readPE(const char * filename);

int main(int argc, const char * argv[])
{
    for(int i = 1; i < argc; i++) {
        readPE(argv[i]);
    }
    return 0;
}

static char * read_full_file(const char * filename) {
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

    return fileMemory;
}

struct Slice {
	const char * ptr;	
	size_t size;
};

const char * readBlob(const char * ptr, uint32_t * len) {
	if ((ptr[0] & 0xE0) == 0xC0) {
		*len = ((*(uint32_t*)ptr) & 0x1FFFFFFF);
		ptr += 3;
	} else if ((ptr[0] & 0xC0) == 0x80) {
		*len = ((*(uint16_t*)ptr) & 0x3FFF);
		ptr += 2;
	} else {
		*len = ((unsigned char)ptr[0] & 0x7F);
		ptr += 1;
	}
	return ptr;
}

struct Heap {
	int count;
	int cap;
	const char ** values;
};

void heap_append(struct Heap * heap, const char * ptr)
{
	if (heap->count >= heap->cap) {
		heap->cap += 64;
		if (heap->values != 0) {
			heap->values = (const char **)realloc(heap->values, sizeof(void*) * heap->cap);
		} else {
			heap->values = (const char **)malloc(sizeof(void*) * heap->cap);
		}

		memset(heap->values + heap->count, 0, sizeof(void*) * (heap->cap - heap->count));
	}

	heap->values[heap->count] = ptr;
	heap->count++;
}

void heap_parse(struct Heap * heap, const char * ptr, size_t len, const char * (*next)(const char * ptr)) {
	const char * begin = ptr;
	const char * end = ptr + len;

	int i;
	const char * ite;
	for(i = 0, ite = begin; ite < end; i++) {
		heap_append(heap, ite);

		ite = next(ite);
	}
}

static const char * next_string(const char * ptr)
{
	return ptr + strlen(ptr) + 1;
}

static const char * next_blob(const char * ptr)
{
	uint32_t len;
	ptr = readBlob(ptr, &len);
	return ptr + len;
}

static const char * next_guid(const char * ptr)
{
	return ptr += 16;
}

static size_t convertUnicodeToUtf8(const char * ptr, size_t len, char * b) {
	size_t rlen = len / 2 * 2;

	const char * begin = b;

	for(size_t i = 0; i < rlen; i += 2) {
		uint16_t c = *(uint16_t*)(ptr + i);

		if (c<0x80) *b++=c;
		else if (c<0x800) *b++=192+c/64, *b++=128+c%64;
		else if (c-0xd800u<0x800) assert(0);
		else if (c<0x10000) *b++=224+c/4096, *b++=128+c/64%64, *b++=128+c%64;
		else if (c<0x110000) *b++=240+c/262144, *b++=128+c/4096%64, *b++=128+c/64%64, *b++=128+c%64;
		else assert(0);
	}

	*b = 0;

	return b - begin;
}

static void dumpHex(const char * ptr, size_t n, const char * sep) {
	for (size_t i = 0; i < n; i++) {
		printf("%s%02X", (i != 0) ? sep : "", (unsigned char)ptr[i]);
	}
	printf("\n");
}

static void dumpStringHeap(struct Heap * heap)
{
	for (int i = 0; i < heap->count; i++) {
		printf("%d: [%s]\n", i, heap->values[i]);
	}
}

static void dumpGUIDHeap(struct Heap * heap)
{
	for (int i = 0; i < heap->count; i++) {
		printf("%d:", i);
		dumpHex(heap->values[i], 16, "-");
	}
}

static void dumpBlobHeap(struct Heap * heap)
{
	for (int i = 0; i < heap->count; i++) {
		const char * ptr = heap->values[i];

		uint32_t len;
		ptr = readBlob(ptr, &len);

		printf("%d:", i);
		dumpHex(ptr, len, " ");
	}

}

static void dumpUnicodeStringHeap(struct Heap * heap)
{
	for (int i = 0; i < heap->count; i++) {
		const char * ptr = heap->values[i];

		uint32_t len;
		ptr = readBlob(ptr, &len);

		char b[256] = {0};
		convertUnicodeToUtf8(ptr, len, b);

		printf("%d: [%s]\n", i, b);
	}
}

static void readPE(const char * filename) {
    char ptr[4] = {0x80, 0x0, 0x0, 0x0};
    assert((*(uint32_t*)ptr) == 128);

    printf("read pe %s\n", filename);

    char * fileMemory = read_full_file(filename);
    if (fileMemory == 0) {
        printf(" %s\n", strerror(errno));
        return;
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
        return; 
    }

    struct Header COFFFileHeader;
    ite = header_init(&COFFFileHeader, TCOFFFileHeader, ite);
    // header_dump(&COFFFileHeader, "COFFFileHeader");

    uint64_t SizeOfOptionalHeader = header_get_field_u64(&COFFFileHeader, "SizeOfOptionalHeader");


    uint64_t CLRRuntimeHeaderVirtualAddress = 0;
    uint64_t CLRRuntimeHeaderSize = 0;

    if (SizeOfOptionalHeader > 0) {
        const char * magic = ite;

        struct Header OptionalHeaderStandardFields;
        struct Header OptionalHeaderWindowsSpecificFields;

        if (magic[0] == 0x0b && magic[1] == 0x01) { // 0x10b
            ite = header_init(&OptionalHeaderStandardFields, TOptionalHeaderStandardFields_PE32, ite);
            ite = header_init(&OptionalHeaderWindowsSpecificFields, TOptionalHeaderWindowsSpecificFields_PE32, ite);
        } else {
            ite = header_init(&OptionalHeaderStandardFields, TOptionalHeaderStandardFields_PE32Plus, ite);
            ite = header_init(&OptionalHeaderWindowsSpecificFields, TOptionalHeaderWindowsSpecificFields_PE32Plus, ite);
        }

        // header_dump(&OptionalHeaderStandardFields, "OptionalHeaderStandardFields");
        // header_dump(&OptionalHeaderWindowsSpecificFields, "OptionalHeaderWindowsSpecificFields");

        uint64_t NumberOfRvaAndSizes = header_get_field_u64(&OptionalHeaderWindowsSpecificFields, "NumberOfRvaAndSizes");

        const char * tableName [] = {
            "Export Table",
            "Import Table",
            "Resource Table",
            "Exception Table",
            "Certificate Table",
            "Base Relocation Table",
            "Debug",
            "Architecture",
            "Global Ptr",
            "TLS Table",
            "Load Config Table",
            "Bound Import",
            "IAT",
            "Delay Import Descriptor",
            "CLR Runtime Header",
            "Reserved",
        };

        for (uint64_t i = 0; i < NumberOfRvaAndSizes; i++) {
            struct Header table;
            ite = header_init(&table, TIMAGE_DATA_DIRECTORY, ite);
            // header_dump(&table, tableName[i]);

            if (strcmp(tableName[i], "CLR Runtime Header") == 0) {
                CLRRuntimeHeaderVirtualAddress = header_get_field_u64(&table, "VirtualAddress");
                CLRRuntimeHeaderSize =  header_get_field_u64(&table, "Size");
            }
        }
    }


    uint64_t NumberOfSections = header_get_field_u64(&COFFFileHeader, "NumberOfSections");
    for(int i = 0; i < NumberOfSections; i++) {
        struct Header SectionTable;
        ite = header_init(&SectionTable, TSectionTable, ite);
        // header_dump(&SectionTable, "SectionTable");


        uint64_t PointerToRawData = header_get_field_u64(&SectionTable, "PointerToRawData");
        uint64_t SizeOfRawData = header_get_field_u64(&SectionTable, "SizeOfRawData");

        const char * sectionPtr = fileMemory + PointerToRawData;

        uint64_t VirtualSize = header_get_field_u64(&SectionTable, "VirtualSize");
        uint64_t VirtualAddress = header_get_field_u64(&SectionTable, "VirtualAddress");

        if (VirtualAddress <= CLRRuntimeHeaderVirtualAddress && VirtualAddress + VirtualSize >= CLRRuntimeHeaderVirtualAddress) {
            printf("Find CLR Header %s\n", header_get_field_str(&SectionTable, "Name"));
            const char * ptr = fileMemory + PointerToRawData + CLRRuntimeHeaderVirtualAddress - VirtualAddress;
            struct Header CLRHeader;
            header_init(&CLRHeader, TCLRHeader, ptr);
            header_dump(&CLRHeader, "CLRHeader");

			uint64_t MetaDataVirtualAddress = header_get_field_u64(&CLRHeader, "MetaDataVirtualAddress");
			uint64_t MetaDataSize = header_get_field_u64(&CLRHeader, "MetaDataSize");

			assert(MetaDataVirtualAddress >= VirtualAddress && MetaDataVirtualAddress < VirtualAddress + VirtualSize);

			ptr = fileMemory + PointerToRawData + MetaDataVirtualAddress - VirtualAddress;

			const char * ptrStartOfTheMetadataRoot = ptr;

			struct Header MetadataRoot_P1;
			ptr = header_init(&MetadataRoot_P1, TMetadataRoot_P1, ptr);
			header_dump(&MetadataRoot_P1, "MetadataRoot_P1");

			uint64_t Length = header_get_field_u64(&MetadataRoot_P1, "Length");
			ptr += Length;

			struct Header MetadataRoot_P2;
            ptr = header_init(&MetadataRoot_P2, TMetadataRoot_P2, ptr);
            header_dump(&MetadataRoot_P2, "MetadataRoot_P2");

			uint64_t Streams = header_get_field_u64(&MetadataRoot_P2, "Streams");

			struct Heap stringHeap = {0, 0, 0};
			struct Heap blobHeap = {0, 0, 0};
			struct Heap guidHeap = {0, 0, 0};
			struct Heap usHeap = {0, 0, 0};

			struct Slice tableStreamSlice = {0, 0};

			for (int j = 0; j < Streams; j++) {
				struct Header StreamHeader;
				ptr = header_init(&StreamHeader, TStreamHeader, ptr);
				header_dump(&StreamHeader, "StreamHeader");

				uint64_t Offset = header_get_field_u64(&StreamHeader, "Offset");
				uint64_t Size = header_get_field_u64(&StreamHeader, "Size");
				const char * Name = header_get_field_str(&StreamHeader, "Name");

				int nameLen = strlen(Name) + 1;
				if (nameLen % 4 != 0) {
					nameLen = nameLen / 4 * 4 + 4;
				}

				ptr += nameLen;

				if (strcmp(Name, "#Strings") == 0) {
					heap_parse(&stringHeap, ptrStartOfTheMetadataRoot + Offset, Size, next_string);
				} else if (strcmp(Name, "#Blob") == 0) {
					heap_parse(&blobHeap, ptrStartOfTheMetadataRoot + Offset, Size, next_blob);
				} else if (strcmp(Name, "#GUID") == 0) {
					heap_parse(&guidHeap, ptrStartOfTheMetadataRoot + Offset, Size, next_guid);
				} else if (strcmp(Name, "#~") == 0) {
					tableStreamSlice.ptr = ptrStartOfTheMetadataRoot + Offset;
					tableStreamSlice.size = Size;
				} else if (strcmp(Name, "#US") == 0) {
					heap_parse(&usHeap, ptrStartOfTheMetadataRoot + Offset, Size, next_blob);
				}
			}

			if (tableStreamSlice.size > 0) {

			}

			dumpStringHeap(&stringHeap);
			dumpBlobHeap(&blobHeap);
			dumpGUIDHeap(&guidHeap);
			dumpUnicodeStringHeap(&usHeap);
        }
    }

    free(fileMemory);
}



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
			}
        }
    }

    free(fileMemory);
}


#ifndef _CLRPARSER_CLR_HEADER_H_
#define _CLRPARSER_CLR_HEADER_H_

#include "table.h"
#include "header_pe.h"

#pragma pack(push, 1)

struct TCLRHeader {
    uint32_t Cb; // Size of the header in bytes

    uint16_t MajorRuntimeVersion; // The minimum version of the runtime required to run this program, currently 2.
    uint16_t MinorRuntimeVersion; // The minor portion of the version, currently 0.

    uint32_t MetaDataVirtualAddress; // RVA and size of the physical metadata (§II.24).
    uint32_t MetaDataSize;           // RVA and size of the physical metadata (§II.24).

    uint32_t Flags; // Flags describing this runtime image. (§II.25.3.3.1).
    uint32_t EntryPointToken; // Token for the MethodDef or File of the entry point for the image

    uint32_t ResourcesVirtualAddress; // RVA and size of implementation-specific resources.
    uint32_t ResourcesSize;           // RVA and size of implementation-specific resources.

    uint32_t StrongNameSignatureVirtualAddress; // RVA of the hash data for this PE file used by the CLI loader for binding and versioning
    uint32_t StrongNameSignatureSize;           // RVA of the hash data for this PE file used by the CLI loader for binding and versioning

    uint64_t CodeManagerTable; // Always 0 (§II.24.1).

    uint32_t VTableFixupsVirtualAddress; // RVA of an array of locations in the file that contain an array of function pointers (e.g., vtable slots), see below.
    uint32_t VTableFixupsSize;           // RVA of an array of locations in the file that contain an array of function pointers (e.g., vtable slots), see below.
    uint64_t ExportAddressTableJumps; //  Always 0 (§II.24.1).
    uint64_t ManagedNativeHeader; // Always 0 (§II.24.1)
};

struct TMetadataRoot_P1 {
    uint32_t Signature;     // Magic signature for physical metadata : 0x424A5342.
    uint16_t MajorVersion;  // Major version, 1 (ignore on read)
    uint16_t MinorVersion;  // Minor version, 1 (ignore on read)
    uint32_t Reserved;      // Reserved, always 0 (§II.24.1).
    uint32_t Length;        // Number of bytes allocated to hold version string (including null terminator), call this x.
                            // Call the length of the string (including the terminator) m (we require m <= 255);
                            // the length x is m rounded up to a multiple of four.
    // 's', 0, Version;     // UTF8-encoded null-terminated version string of length m (see above)
};


struct TMetadataRoot_P2 {
	uint16_t Flags; // Reserved, always 0 (§II.24.1).
	uint16_t Streams; // Number of streams, say n.
};

struct TStreamHeader {
	uint32_t Offset; // Memory offset to start of this stream from start of the metadata root (§II.24.2.1)
	uint32_t Size;   // Size of this stream in bytes, shall be a multiple of 4.
	// {'s', 0, Name;   //  Name of the stream as null-terminated variable length array of ASCII characters, padded to the next 4-byte boundary with \0 characters. The name is limited to 32 characters.
};

struct TMetadataTableHeader {
    uint32_t Reserved1; // Reserved, always 0 (§II.24.1).
    uint8_t MajorVersion; // Major version of table schemata; shall be 2 (§II.24.1).
    uint8_t MinorVersion; // Minor version of table schemata; shall be 0 (§II.24.1).
    uint8_t HeapSizes; // Bit vector for heap sizes.
    uint8_t Reserved2; // Reserved, always 1 (§II.24.1).
    uint64_t Valid;  // Bit vector of present tables, let n be the number of bits that are 1.
    uint64_t Sorted; // Bit vector of sorted tables.
    // {'u', 0, Rows;             // Array of n 4-byte unsigned integers indicating the number of rows for each present table.
    // 24+4*n Tables The sequence of physical tables.
    // {0, 0, 0},
};

#pragma pack(pop)


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

#endif

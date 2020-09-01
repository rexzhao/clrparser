#ifndef _CLRPARSER_CLR_HEADER_H_
#define _CLRPARSER_CLR_HEADER_H_

#include "table.h"

const struct FieldInfo TCLRHeader [] = {
    {'u', 4, "Cb"}, // Size of the header in bytes

    {'u', 2, "MajorRuntimeVersion"}, // The minimum version of the runtime required to run this program, currently 2.
    {'u', 2, "MinorRuntimeVersion"}, // The minor portion of the version, currently 0.

    {'x', 4, "MetaDataVirtualAddress"}, // RVA and size of the physical metadata (§II.24).
    {'x', 4, "MetaDataSize"},           // RVA and size of the physical metadata (§II.24).

    {'x', 4, "Flags"}, // Flags describing this runtime image. (§II.25.3.3.1).
    {'x', 4, "EntryPointToken"}, // Token for the MethodDef or File of the entry point for the image

    {'x', 4, "ResourcesVirtualAddress"}, // RVA and size of implementation-specific resources.
    {'x', 4, "ResourcesSize"},           // RVA and size of implementation-specific resources.

    {'x', 4, "StrongNameSignatureVirtualAddress"}, // RVA of the hash data for this PE file used by the CLI loader for binding and versioning
    {'x', 4, "StrongNameSignatureSize"},           // RVA of the hash data for this PE file used by the CLI loader for binding and versioning

    {'u', 8, "CodeManagerTable"}, // Always 0 (§II.24.1).

    {'x', 4, "VTableFixupsVirtualAddress"}, // RVA of an array of locations in the file that contain an array of function pointers (e.g., vtable slots), see below.
    {'x', 4, "VTableFixupsSize"},           // RVA of an array of locations in the file that contain an array of function pointers (e.g., vtable slots), see below.
    {'u', 8, "ExportAddressTableJumps"}, //  Always 0 (§II.24.1).
    {'u', 8, "ManagedNativeHeader"}, // Always 0 (§II.24.1)
    {0, 0, 0},
};

const struct FieldInfo TMetadataRoot_P1 [] = {
    {'x', 4, "Signature"},     // Magic signature for physical metadata : 0x424A5342.
    {'u', 2, "MajorVersion"},  // Major version, 1 (ignore on read)
    {'u', 2, "MinorVersion"},  // Minor version, 1 (ignore on read)
    {'u', 4, "Reserved"},      // Reserved, always 0 (§II.24.1).
    {'u', 4, "Length"},        // Number of bytes allocated to hold version string (including null terminator), call this x.
                               // Call the length of the string (including the terminator) m (we require m <= 255);
                               // the length x is m rounded up to a multiple of four.
    {'s', 0, "Version"},       // UTF8-encoded null-terminated version string of length m (see above)

	{0, 0, 0},
};


const struct FieldInfo TMetadataRoot_P2 [] = {
	{'x', 2, "Flags"}, // Reserved, always 0 (§II.24.1).
	{'u', 2, "Streams"}, // Number of streams, say n.

	{0, 0, 0},
};

const struct FieldInfo TStreamHeader [] = {
	{'u', 4, "Offset"}, // Memory offset to start of this stream from start of the metadata root (§II.24.2.1)
	{'u', 4, "Size"},   // Size of this stream in bytes, shall be a multiple of 4.
	{'s', 0, "Name"},   //  Name of the stream as null-terminated variable length array of ASCII characters, padded to the next 4-byte boundary with \0 characters. The name is limited to 32 characters.
	{0, 0, 0},
};

const struct FieldInfo TMetadataTableHeader [] = {
    {'u', 4, "Reserved"}, // Reserved, always 0 (§II.24.1).
    {'u', 1, "MajorVersion"}, // Major version of table schemata; shall be 2 (§II.24.1).
    {'u', 1, "MinorVersion"}, // Minor version of table schemata; shall be 0 (§II.24.1).
    {'u', 1, "HeapSizes"}, // Bit vector for heap sizes.
    {'u', 1, "Reserved"}, // Reserved, always 1 (§II.24.1).
    {'x', 8, "Valid"},  // Bit vector of present tables, let n be the number of bits that are 1.
    {'x', 8, "Sorted"}, // Bit vector of sorted tables.
    // {'u', 0, "Rows"},             // Array of n 4-byte unsigned integers indicating the number of rows for each present table.
    // 24+4*n Tables The sequence of physical tables.
    {0, 0, 0},
};


#endif

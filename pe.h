#ifndef _CLRPARSER_PE_HEADER_H_
#define _CLRPARSER_PE_HEADER_H_

#include <stdlib.h>

#pragma pack(push, 1)
struct TCOFFFileHeader {
    uint16_t Machine;                // The number that identifies the type of target machine. For more information, see Machine Types.
    uint16_t NumberOfSections;       // The number of sections. This indicates the size of the section table, which immediately follows the headers.
    uint32_t TimeDateStamp;          // The low 32 bits of the number of seconds since 00:00 January 1, 1970 (a C run-time time_t value), which indicates when the file was created.
    uint32_t PointerToSymbolTable;   // The file offset of the COFF symbol table, or zero if no COFF symbol table is present. 
    uint32_t NumberOfSymbols;        //   The number of entries in the symbol table. 
    uint16_t SizeOfOptionalHeader;   // The size of the optional header, which is required for executable files but not for object files. 
    uint16_t Characteristics;        // The flags that indicate the attributes of the file. For specific flag values, see Characteristics.
};


struct TOptionalHeaderStandardFields_PE32 { // (Image Only)
    uint16_t Magic;                    // The unsigned integer that identifies the state of the image file.
                                       //   The most common number is 0x10B, which identifies it as a normal executable file. 
                                       //   0x107 identifies it as a ROM image, and 0x20B identifies it as a PE32+ executable.
    uint8_t MajorLinkerVersion;        // The linker major version number.
    uint8_t MinorLinkerVersion;        // The linker minor version number.
    uint32_t SizeOfCode;               // The size of the code (text) section, or the sum of all code sections if there are multiple sections.
    uint32_t SizeOfInitializedData;    // The size of the initialized data section, or the sum of all such sections if there are multiple data sections.
    uint32_t SizeOfUninitializedData;  // The size of the uninitialized data section (BSS), or the sum of all such sections if there are multiple BSS sections.
    uint32_t AddressOfEntryPoint;      // The address of the entry point relative to the image base when the executable file is loaded into memory. 
                                       //   For program images, this is the starting address. 
                                       //   For device drivers, this is the address of the initialization function. 
                                       //   An entry point is optional for DLLs. When no entry point is present, this field must be zero.

    uint32_t BaseOfCode; // The address that is relative to the image base of the beginning-of-code section when it is loaded into memory.
    uint32_t BaseOfData; // The address that is relative to the image base of the beginning-of-data section when it is loaded into memory.
};

struct TOptionalHeaderWindowsSpecificFields_PE32 { 
    uint32_t ImageBase; // The preferred address of the first byte of image when loaded into memory; must be a multiple of 64 K.
                        //   The default for DLLs is 0x10000000. 
                        //   The default for Windows CE EXEs is 0x00010000. 
                        //   The default for Windows NT, Windows 2000, Windows XP, Windows 95, Windows 98, and Windows Me is 0x00400000.
    uint32_t SectionAlignment;  // The alignment (in bytes) of sections when they are loaded into memory.
                                //   It must be greater than or equal to FileAlignment.
                                //   The default is the page size for the architecture.
    uint32_t FileAlignment; // The alignment factor (in bytes) that is used to align the raw data of sections in the image file. 
                            //   The value should be a power of 2 between 512 and 64 K, inclusive. The default is 512.
                            //   If the SectionAlignment is less than the architecture's page size, then FileAlignment must match SectionAlignment.

    uint16_t MajorOperatingSystemVersion; // The major version number of the required operating system.
    uint16_t MinorOperatingSystemVersion; // The minor version number of the required operating system.

    uint16_t MajorImageVersion; // The major version number of the image.
    uint16_t MinorImageVersion; // The minor version number of the image.

    uint16_t MajorSubsystemVersion; // The major version number of the subsystem.
    uint16_t MinorSubsystemVersion; // The minor version number of the subsystem.

    uint32_t Win32VersionValue;   // Reserved, must be zero.
    uint32_t SizeOfImage;         // The size (in bytes) of the image, including all headers, as the image is loaded in memory. It must be a multiple of SectionAlignment.
    uint32_t SizeOfHeaders;       // The combined size of an MS-DOS stub, PE header, and section headers rounded up to a multiple of FileAlignment.
    uint32_t CheckSum;            // The image file checksum. 
                                  //   The algorithm for computing the checksum is incorporated into IMAGHELP.DLL.
                      //   The following are checked for validation at load time: all drivers, any DLL loaded at boot time, 
                      //   and any DLL that is loaded into a critical Windows process.
    uint16_t Subsystem;           // The subsystem that is required to run this image. For more information, see Windows Subsystem.
    uint16_t DllCharacteristics;  // For more information, see DLL Characteristics later in this specification.
    uint32_t SizeOfStackReserve;  // The size of the stack to reserve. Only SizeOfStackCommit is committed; 
                                  // the rest is made available one page at a time until the reserve size is reached.
    uint32_t SizeOfStackCommit;   // The size of the stack to commit.
    uint32_t SizeOfHeapReserve;   // The size of the local heap space to reserve. 
                                  //   Only SizeOfHeapCommit is committed", the rest is made available one page at a time until the reserve size is reached.
    uint32_t SizeOfHeapCommit;    // The size of the local heap space to commit.
    uint32_t LoaderFlags;         // Reserved, must be zero.
    uint32_t NumberOfRvaAndSizes; // The number of data-directory entries in the remainder of the optional header. Each describes a location and size.
};

struct TOptionalHeaderStandardFields_PE32Plus { // (Image Only)
    uint16_t Magic;                    // The unsigned integer that identifies the state of the image file.
                                       //   The most common number is 0x10B, which identifies it as a normal executable file. 
                               //   0x107 identifies it as a ROM image, and 0x20B identifies it as a PE32+ executable.
    uint8_t MajorLinkerVersion;        // The linker major version number.
    uint8_t MinorLinkerVersion;        // The linker minor version number.
    uint32_t SizeOfCode;               // The size of the code (text) section, or the sum of all code sections if there are multiple sections.
    uint32_t SizeOfInitializedData;    // The size of the initialized data section, or the sum of all such sections if there are multiple data sections.
    uint32_t SizeOfUninitializedData;  // The size of the uninitialized data section (BSS), or the sum of all such sections if there are multiple BSS sections.
    uint32_t AddressOfEntryPoint;      // The address of the entry point relative to the image base when the executable file is loaded into memory. 
                                       //   For program images, this is the starting address. 
                       //   For device drivers, this is the address of the initialization function. 
                       //   An entry point is optional for DLLs. When no entry point is present, this field must be zero.

    uint32_t BaseOfCode; // The address that is relative to the image base of the beginning-of-code section when it is loaded into memory.
};

struct TOptionalHeaderWindowsSpecificFields_PE32Plus {
    uint64_t ImageBase; // The preferred address of the first byte of image when loaded into memory; must be a multiple of 64 K.
                            //   The default for DLLs is 0x10000000. 
                //   The default for Windows CE EXEs is 0x00010000. 
                //   The default for Windows NT, Windows 2000, Windows XP, Windows 95, Windows 98, and Windows Me is 0x00400000.
    uint32_t SectionAlignment; // The alignment (in bytes) of sections when they are loaded into memory.
                                   //   It must be greater than or equal to FileAlignment.
                   //   The default is the page size for the architecture.
    uint32_t FileAlignment; // The alignment factor (in bytes) that is used to align the raw data of sections in the image file. 
                            //   The value should be a power of 2 between 512 and 64 K, inclusive. The default is 512.
                //   If the SectionAlignment is less than the architecture's page size, then FileAlignment must match SectionAlignment.
                //
    uint16_t MajorOperatingSystemVersion; // The major version number of the required operating system.
    uint16_t MinorOperatingSystemVersion; // The minor version number of the required operating system.

    uint16_t MajorImageVersion; // The major version number of the image.
    uint16_t MinorImageVersion; // The minor version number of the image.

    uint16_t MajorSubsystemVersion; // The major version number of the subsystem.
    uint16_t MinorSubsystemVersion; // The minor version number of the subsystem.

    uint32_t Win32VersionValue;   // Reserved, must be zero.
    uint32_t SizeOfImage;         // The size (in bytes) of the image, including all headers, as the image is loaded in memory. It must be a multiple of SectionAlignment.
    uint32_t SizeOfHeaders;       // The combined size of an MS-DOS stub, PE header, and section headers rounded up to a multiple of FileAlignment.
    uint32_t CheckSum;            // The image file checksum. 
                                  //   The algorithm for computing the checksum is incorporated into IMAGHELP.DLL.
                      //   The following are checked for validation at load time: all drivers, any DLL loaded at boot time, 
                      //   and any DLL that is loaded into a critical Windows process.
    uint16_t Subsystem;           // The subsystem that is required to run this image. For more information, see Windows Subsystem.
    uint16_t DllCharacteristics;  // For more information, see DLL Characteristics later in this specification.
    uint64_t SizeOfStackReserve;  // The size of the stack to reserve. Only SizeOfStackCommit is committed; 
                                  // the rest is made available one page at a time until the reserve size is reached.
    uint64_t SizeOfStackCommit;   // The size of the stack to commit.
    uint64_t SizeOfHeapReserve;   // The size of the local heap space to reserve. 
                                  //   Only SizeOfHeapCommit is committed", the rest is made available one page at a time until the reserve size is reached.
    uint64_t SizeOfHeapCommit;    // The size of the local heap space to commit.

    uint32_t LoaderFlags;         // Reserved, must be zero.
    uint32_t NumberOfRvaAndSizes; // The number of data-directory entries in the remainder of the optional header. Each describes a location and size.
};

struct TIMAGE_DATA_DIRECTORY {
    uint32_t VirtualAddress;
    uint32_t Size;
};

struct TSectionTable {
    char Name[8]; // An 8-byte, null-padded UTF-8 encoded string.
                  //   If the string is exactly 8 characters long, there is no terminating null. 
                  //   For longer names, this field contains a slash (/) that is followed by an ASCII representation of a decimal number that is an offset into the string table. 
                  //   Executable images do not use a string table and do not support section names longer than 8 characters. 
                  //   Long names in object files are truncated if they are emitted to an executable file.
    uint32_t VirtualSize; // The total size of the section when loaded into memory.
                          //   If this value is greater than SizeOfRawData, the section is zero-padded. 
                          //   This field is valid only for executable images and should be set to zero for object files.
    uint32_t VirtualAddress; // For executable images, the address of the first byte of the section relative to the image base when the section is loaded into memory. 
                             //   For object files, this field is the address of the first byte before relocation is applied; 
                             //   for simplicity, compilers should set this to zero. Otherwise, it is an arbitrary value that is subtracted from offsets during relocation.
    uint32_t SizeOfRawData;  // The size of the section (for object files) or the size of the initialized data on disk (for image files). 
                             //   For executable images, this must be a multiple of FileAlignment from the optional header.
                             //   If this is less than VirtualSize, the remainder of the section is zero-filled.
                             //   Because the SizeOfRawData field is rounded but the VirtualSize field is not, it is possible for SizeOfRawData to be greater than VirtualSize as well. 
                             //   When a section contains only uninitialized data, this field should be zero.
    uint32_t PointerToRawData; // The file pointer to the first page of the section within the COFF file. 
                               //   For executable images, this must be a multiple of FileAlignment from the optional header. 
                               //   For object files, the value should be aligned on a 4-byte boundary for best performance. 
                               //   When a section contains only uninitialized data, this field should be zero.
    uint32_t PointerToRelocations; // The file pointer to the beginning of relocation entries for the section. This is set to zero for executable images or if there are no relocations.
    uint32_t PointerToLinenumbers; // The file pointer to the beginning of line-number entries for the section. This is set to zero if there are no COFF line numbers. 
                                   // This value should be zero for an image because COFF debugging information is deprecated.
    uint16_t NumberOfRelocations; // The number of relocation entries for the section. This is set to zero for executable images.
    uint16_t NumberOfLinenumbers; // The number of line-number entries for the section. This value should be zero for an image because COFF debugging information is deprecated.
    uint32_t Characteristics; // The flags that describe the characteristics of the section. For more information, see Section Flags.
};

#pragma pack(pop)

struct PEFile {
	char * ptr;
	size_t size;

	struct TCOFFFileHeader * COFFFileHeader;

	struct TOptionalHeaderStandardFields_PE32 * OptionalHeaderStandardFields_PE32;
	struct TOptionalHeaderWindowsSpecificFields_PE32 * OptionalHeaderWindowsSpecificFields_PE32;

	struct TOptionalHeaderStandardFields_PE32Plus * OptionalHeaderStandardFields_PE32Plus;
	struct TOptionalHeaderWindowsSpecificFields_PE32Plus * OptionalHeaderWindowsSpecificFields_PE32Plus;

	struct TIMAGE_DATA_DIRECTORY * IMAGE_DATA_DIRECTORY;
	struct TSectionTable * SectionTable;
};

int read_pe_file(struct PEFile * file, const char * filename);
const char * find_virtual_addr(struct PEFile * file, uint64_t VirtualAddress);

#endif
